def encode_by_type(v):
    if isinstance(v, int):
        return encode_int(v)
    elif isinstance(v, str):
        return encode_string(v)
    else:
        return encode_dict(v)


def decode_by_type(v, pos):
    if v[pos] == 'i':
        return decode_int(v, pos)
    elif v[pos] == 'd':
        return decode_dict(v, pos)
    else:
        return decode_string(v, pos)


def encode_int(n):
    s = 'i'
    s += str(n)
    s += 'e'
    return s


def decode_int(v, pos):
    pos += 1
    e_pos = v.index('e', pos)
    n = int(v[pos:e_pos])
    return n, e_pos + 1


def encode_string(w):
    s = str(len(w))
    s += ':'
    s += w
    return s


def decode_string(v, pos):
    colon_pos = v.index(':', pos)
    n = int(v[pos:colon_pos])
    colon_pos += 1
    return v[colon_pos:colon_pos + n], colon_pos + n


def encode_dict(d):
    s = 'd'
    item_list = list(d.items())
    item_list.sort()
    for k, v in item_list:
        s += str(len(k))
        s += ':'
        s += k
        s += encode_by_type(v)
    s += 'e'
    return s


def decode_dict(v, pos):
    d = {}
    pos += 1
    while v[pos] != 'e':
        k, pos = decode_string(v, pos)
        d[k], pos = decode_by_type(v, pos)
    return d, pos + 1


def bencode(v):
    s = encode_by_type(v)
    return s


def bdecode(v):
    s, l = decode_by_type(v, 0)
    return s
