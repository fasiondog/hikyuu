#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Python side wrapper of the python part bridge: validate the arguments before handing them to the
# C++ registry, so that a malformed part is reported with a clear message.

from .core import part

__all__ = ['register_python_part']


def _check_params(meta, full_name):
    names = set()
    for param in meta.params:
        if not param.name:
            raise ValueError('Empty param name in part "{}"!'.format(full_name))
        if param.name in names:
            raise ValueError(
                'Duplicated param name "{}" in part "{}"!'.format(param.name, full_name)
            )
        names.add(param.name)

        if param.required and param.default_value:
            raise ValueError(
                'The param "{}" of part "{}" is required, it must not have a default value!'.format(
                    param.name, full_name
                )
            )

        if param.is_list and not param.is_part_param:
            raise ValueError(
                'The param "{}" of part "{}" is not a part param, it can not be a list!'.format(
                    param.name, full_name
                )
            )


def register_python_part(full_name, meta, factory, overwrite=False):
    """Register a python defined part into the built-in registry

    :param str full_name: the part full name, in the form "core.<type>.<name>"
    :param PartMeta meta: the part meta info, whose type must match the type in the full name
    :param factory: the callable factory(scalars, parts) -> part
    :param bool overwrite: replace an existing registration (built-in, plugin or python)
    """
    name_parts = full_name.split('.') if isinstance(full_name, str) else []
    if len(name_parts) != 3 or name_parts[0].lower() != 'core':
        raise ValueError(
            'Invalid part full name: "{}", expected the form "core.<type>.<name>"!'.format(full_name)
        )

    if not isinstance(meta, part.PartMeta):
        raise ValueError('The meta of part "{}" must be a PartMeta!'.format(full_name))

    part_type = part.PartType.__members__.get(name_parts[1].upper())
    if part_type is None:
        raise ValueError(
            'Invalid part type "{}" in the part full name "{}"!'.format(name_parts[1], full_name)
        )

    if not isinstance(meta.type, part.PartType) or meta.type != part_type:
        raise ValueError(
            'The meta type "{}" does not match the part full name "{}"!'.format(meta.type, full_name)
        )

    if not meta.name:
        raise ValueError('Empty meta name of part "{}"!'.format(full_name))

    _check_params(meta, full_name)

    if not callable(factory):
        raise ValueError('The factory of part "{}" is not callable!'.format(full_name))

    part._register_python_part(full_name, meta, factory, overwrite)
