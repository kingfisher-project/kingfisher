"""Sphinx extension that documents the input file schema.

The ``schema-block`` directive renders a table of the options in one block of
the JSON Schema named by the ``schema_path`` config value::

    ```{schema-block} materials
    ```

The argument is a dotted path of property names, such as ``solver.source_iterations``.
Descriptions come from the schema, so the tables follow it when it changes.
The build warns when a top-level block of the schema is not documented
anywhere, and fails on schema keywords that this extension can't render.
"""

import json
from pathlib import Path

from docutils import nodes
from sphinx.util import logging
from sphinx.util.docutils import SphinxDirective

logger = logging.getLogger(__name__)

# Keywords that change what a valid file looks like and that are not rendered.
# Failing on them keeps the tables from silently going out of date.
_UNSUPPORTED = {
    "oneOf",
    "anyOf",
    "not",
    "else",
    "patternProperties",
    "dependentRequired",
    "dependentSchemas",
    "prefixItems",
    "contains",
}

_JSON_TYPES = {str: "string", bool: "boolean", int: "integer", float: "number"}

_HEADER = ["Field", "Type", "Required", "Constraints", "Description"]
_WIDTHS = [20, 18, 14, 24, 40]


class SchemaDocError(Exception):
    """The schema uses something this extension can't document."""


def _resolve(schema, node):
    """Follows ``$ref`` links. Keywords next to a ``$ref`` take precedence."""
    while "$ref" in node:
        ref = node["$ref"]
        if not ref.startswith("#/"):
            raise SchemaDocError(f"Only local $ref links are supported, got {ref!r}.")
        target = schema
        for part in ref[2:].split("/"):
            target = target[part]
        node = {**target, **{k: v for k, v in node.items() if k != "$ref"}}
    unsupported = _UNSUPPORTED & node.keys()
    if unsupported:
        raise SchemaDocError(
            f"Schema keyword(s) {sorted(unsupported)} can't be documented yet; "
            "add support in docs/source/_ext/schema_doc.py."
        )
    return node


def _literal(value):
    text = str(value).lower() if isinstance(value, bool) else str(value)
    return f"`{text}`"


def _type_name(schema, node):
    node = _resolve(schema, node)
    kind = node.get("type")
    if kind == "array":
        if "items" in node:
            return f"array[{_type_name(schema, node['items'])}]"
        return "array"
    if isinstance(kind, list):
        return " | ".join(kind)
    if kind:
        return kind
    values = node.get("enum", [node["const"]] if "const" in node else [])
    kinds = sorted({_JSON_TYPES.get(type(value), "any") for value in values})
    return " | ".join(kinds) if kinds else "any"


def _value_constraints(node, prefix=""):
    out = []
    if "const" in node:
        out.append(f"{prefix}must be {_literal(node['const'])}")
    if "enum" in node:
        out.append(f"{prefix}one of " + ", ".join(_literal(v) for v in node["enum"]))
    for key, symbol in (
        ("minimum", ">="),
        ("exclusiveMinimum", ">"),
        ("maximum", "<="),
        ("exclusiveMaximum", "<"),
    ):
        if key in node:
            out.append(f"{prefix}{symbol} {node[key]}")
    if "minLength" in node:
        out.append(f"{prefix}at least {node['minLength']} character(s)")
    if "maxLength" in node:
        out.append(f"{prefix}at most {node['maxLength']} character(s)")
    if "pattern" in node:
        out.append(f"{prefix}matches {_literal(node['pattern'])}")
    return out


def _constraints(schema, node, depth=1):
    """Lists the restrictions on a value, one string per restriction."""
    node = _resolve(schema, node)
    if node.get("type") != "array":
        return _value_constraints(node, "" if depth == 1 else "each value: ")
    tag = "" if depth == 1 else f"level {depth}: "
    out = []
    if "minItems" in node:
        out.append(f"{tag}at least {node['minItems']} item(s)")
    if "maxItems" in node:
        out.append(f"{tag}at most {node['maxItems']} item(s)")
    if node.get("uniqueItems"):
        out.append(f"{tag}items are unique")
    items = _resolve(schema, node["items"]) if "items" in node else {}
    if items.get("type") == "array":
        out += _constraints(schema, items, depth + 1)
    elif items.get("type") != "object":
        out += _value_constraints(items, "each value: ")
    return out


def _conditionally_required(schema, node):
    """Maps a property name to the condition under which it is required.

    Only the form used by the schema is understood: an ``allOf`` of
    ``if``/``then`` pairs, where ``if`` tests one property for a ``const`` and
    ``then`` lists required properties.
    """
    out = {}
    for clause in node.get("allOf", []):
        condition = clause.get("if", {})
        then = clause.get("then", {})
        props = condition.get("properties", {})
        ok = (
            set(clause) == {"if", "then"}
            and set(condition) == {"properties"}
            and set(then) == {"required"}
            and len(props) == 1
            and set(next(iter(props.values()))) == {"const"}
        )
        if not ok:
            raise SchemaDocError(
                "Only allOf clauses of the form if: {properties: {x: {const: ...}}}, "
                "then: {required: [...]} can be documented."
            )
        (key, test), = props.items()
        text = f"when {_literal(key)} is {_literal(test['const'])}"
        for name in then["required"]:
            out.setdefault(name, []).append(text)
    return {name: "; ".join(texts) for name, texts in out.items()}


def _row(schema, path, node, requirement):
    return (
        f"`{path}`",
        f"`{_type_name(schema, node)}`",
        requirement,
        _constraints(schema, node),
        node.get("description", ""),
    )


def _rows(schema, node, prefix, rows):
    """Appends one row per property of an object, recursing into nested objects."""
    node = _resolve(schema, node)
    required = set(node.get("required", []))
    conditional = _conditionally_required(schema, node)
    for name, sub in node.get("properties", {}).items():
        sub = _resolve(schema, sub)
        path = prefix + name
        if name in required:
            requirement = "yes"
        else:
            requirement = conditional.get(name, "no")
        rows.append(_row(schema, path, sub, requirement))
        child, suffix = sub, ""
        if sub.get("type") == "array" and "items" in sub:
            items = _resolve(schema, sub["items"])
            if items.get("type") == "object":
                child, suffix = items, "[]"
        if child.get("type") == "object":
            _rows(schema, child, f"{path}{suffix}.", rows)


def _inline(text):
    """Splits text on backticks into plain text and literals."""
    out = []
    for i, part in enumerate(text.split("`")):
        if part:
            out.append(nodes.literal(part, part) if i % 2 else nodes.Text(part))
    return out


def _cell(texts, classes=()):
    """A table cell holding one paragraph per string."""
    return [nodes.paragraph("", "", *_inline(text), classes=list(classes)) for text in texts]


def _table(rows):
    table = nodes.table(classes=["kf-schema-table"])
    tgroup = nodes.tgroup(cols=len(_HEADER))
    table += tgroup
    for width in _WIDTHS:
        tgroup += nodes.colspec(colwidth=width)

    def row(cells):
        out = nodes.row()
        for cell in cells:
            entry = nodes.entry()
            entry += cell
            out += entry
        return out

    head = nodes.thead()
    head += row([_cell([title]) for title in _HEADER])
    tgroup += head
    body = nodes.tbody()
    for field, kind, requirement, constraints, description in rows:
        classes = ["kf-required"] if requirement == "yes" else []
        body += row(
            [
                _cell([field]),
                _cell([kind]),
                _cell([requirement], classes),
                _cell(constraints),
                _cell([description] if description else []),
            ]
        )
    tgroup += body
    return table


class SchemaBlock(SphinxDirective):
    """Renders the options of one block of the input schema as a table."""

    required_arguments = 1
    final_argument_whitespace = False

    def run(self):
        path = Path(self.config.schema_path)
        self.env.note_dependency(str(path))
        schema = json.loads(path.read_text())

        names = self.arguments[0].split(".")
        blocks = self.env.__dict__.setdefault("schema_doc_blocks", {})
        blocks.setdefault(self.env.docname, set()).add(names[0])

        try:
            node = schema
            for name in names:
                parent = _resolve(schema, node)
                if "properties" not in parent or name not in parent["properties"]:
                    raise SchemaDocError(
                        f"The schema has no block {self.arguments[0]!r} "
                        f"(no property {name!r})."
                    )
                node = parent["properties"][name]
            node = _resolve(schema, node)
            items = _resolve(schema, node["items"]) if "items" in node else {}
            if node.get("type") == "array" and items.get("type") == "object":
                node = items
            rows = []
            if node.get("type") == "object":
                _rows(schema, node, "", rows)
            else:
                # A single value, such as `version`: one row for the value itself.
                requirement = "yes" if name in parent.get("required", []) else "no"
                rows.append(_row(schema, name, node, requirement))
        except SchemaDocError as error:
            raise self.error(str(error)) from error
        return [_table(rows)]


def _purge(app, env, docname):
    getattr(env, "schema_doc_blocks", {}).pop(docname, None)


def _merge(app, env, docnames, other):
    env.__dict__.setdefault("schema_doc_blocks", {}).update(
        getattr(other, "schema_doc_blocks", {})
    )


def _check_coverage(app, env):
    schema = json.loads(Path(app.config.schema_path).read_text())
    documented = set().union(*getattr(env, "schema_doc_blocks", {}).values())
    for name in schema.get("properties", {}):
        if name not in documented:
            logger.warning(
                "The input schema block %r is not documented; add a "
                "'{schema-block} %s' directive to the input file page.",
                name,
                name,
            )


def setup(app):
    app.add_config_value("schema_path", "", "env", types=[str])
    app.add_directive("schema-block", SchemaBlock)
    app.connect("env-purge-doc", _purge)
    app.connect("env-merge-info", _merge)
    app.connect("env-check-consistency", _check_coverage)
    return {"version": "0.1", "parallel_read_safe": True, "parallel_write_safe": True}
