import re

def is_float_with_exp(s):
    pattern = r'^[+-]?[0-9]*((\.[0-9]+|[0-9]+))[E|e][+-]?[0-9]+$'
    return bool(re.match(pattern, s))

print(f"{is_float_with_exp('2.3e4')}")
print(f"{is_float_with_exp('.3e4')}")
print(f"{is_float_with_exp('+2.3e4')}")
print(f"{is_float_with_exp('-2.3e4')}")
print(f"{is_float_with_exp('2.3')}")
print(f"{is_float_with_exp('2.e4')}")


