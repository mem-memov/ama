## @file
# @brief Validate the explicit evidence-record schema without inventing missing fields or accepting duplicate JSON keys.
import json


def unique_object(pairs):
    """Reject duplicate keys before dictionary construction can silently discard an obligation."""
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError('duplicate evidence key: ' + key)
        result[key] = value
    return result


def load(path):
    """Read one exact UTF-8 evidence manifest; malformed data is never repaired or defaulted."""
    return json.loads(path.read_text(encoding='utf-8'), object_pairs_hook=unique_object,
                      parse_constant=nonstandard_constant)


def nonstandard_constant(value):
    """Reject non-JSON NaN/Infinity tokens rather than accepting the interpreter's extension."""
    raise ValueError('nonstandard JSON constant: ' + value)


def fields(value, expected, context):
    """Require exactly the declared object fields, keeping typos and accidental extensions visible."""
    if not isinstance(value, dict) or set(value) != set(expected):
        raise ValueError('invalid fields for ' + context)


def text(value, context):
    """Require a substantive explicit string rather than coercing arbitrary JSON values."""
    if not isinstance(value, str) or not value.strip() or '\0' in value:
        raise ValueError('missing text for ' + context)
    value.encode('utf-8')


def strings(value, context):
    """Validate an explicitly nonempty unique sequence of exact identifiers or paths."""
    if not isinstance(value, list) or not value:
        raise ValueError('missing entries for ' + context)
    for item in value:
        text(item, context)
    if len(set(value)) != len(value):
        raise ValueError('duplicate entries for ' + context)


def record(value, context, groups, complete):
    """A covered claim needs explicit reasoning and known evidence groups; an open claim always explains its gap."""
    fields(value, ('status', 'argument', 'groups'), context)
    if value['status'] not in ('covered', 'open'):
        raise ValueError('invalid evidence status for ' + context)
    text(value['argument'], context)
    strings(value['groups'], context)
    if not set(value['groups']) <= set(groups):
        raise ValueError('unknown evidence group for ' + context)
    if complete and value['status'] != 'covered':
        raise ValueError('open conformance obligation: ' + context)
