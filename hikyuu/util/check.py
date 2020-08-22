#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#==========================================================
# History
# 1. 20200821, Added by fasiondog
#==========================================================


class CheckError(Exception):
    def __init__(self, expression, message):
        self.expression = expression
        self.message = message

    def __str__(self):
        return self.message


def checkif(expression, message, excepion=None, **kwargs):
    if expression:
        if excepion is None:
            raise CheckError(expression, message)
        else:
            raise excepion(message, **kwargs)
