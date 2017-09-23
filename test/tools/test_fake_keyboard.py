#!/usr/bin/env python3

from time import sleep
from socket import socket


def checksum(s):
    if s[0] == '$':
        s = s[1:]
    ret = ord(s[0])
    for c in s[1:]:
        ret ^= ord(c)
    return ret


def make_gps_msg(header, body):
    m = '${},{}'.format(header, body)
    cksum = checksum(m)
    m = '{}*{}'.format(m, "%X" % cksum)
    return m


def make_scan_msg(code):
    return make_gps_msg('FAKBD', hex(code)[2:])


def make_time_msg():
    return make_gps_msg('GPRMC', '091717,A,5103.5403,N,00741.5742,E,055.3,022.4,230610,000.3,W')

# Read all the KEY_* definitions from the include file
for l in open('/usr/include/linux/input-event-codes.h').readlines():
    if not l.startswith('#define KEY_'):
        continue
    parts = l.split()
    #print("got {!r}".format(parts))
    define, key, value = parts[:3]
    if value == '(KEY_MAX+1)':
        value = globals()['KEY_MAX'] + 1
    else:
        value = (int(value, 16) if value.startswith('0x') else int(value)) if value not in globals() else globals()[value]
    globals()[key] = value


key_to_scan = [
    0, # 0
    0, # 1
    0, # 2
    0, # 3
    0, # 4
    0, # 5
    0, # 6
    0, # 7
    0, # 8
    KEY_TAB, # 9
    KEY_ENTER, # 10
    0, # 11
    0, # 12
    KEY_ENTER, # 13
    0, # 14
    0, # 15
    0, # 16
    0, # 17
    0, # 18
    0, # 19
    0, # 20
    0, # 21
    0, # 22
    0, # 23
    0, # 24
    0, # 25
    0, # 26
    KEY_ESC, # 27
    0, # 28
    0, # 29
    0, # 30
    0, # 31
    0, # 32
    0, # 33
    0, # 34
    0, # 35
    0, # 36
    0, # 37
    0, # 38
    0, # 39
    0, # 40
    0, # 41
    0, # 42
    0, # 43
    0, # 44
    0, # 45
    0, # 46
    0, # 47
    KEY_0, # 48
    KEY_1, # 49
    KEY_2, # 50
    KEY_3, # 51
    KEY_4, # 52
    KEY_5, # 53
    KEY_6, # 54
    KEY_7, # 55
    KEY_8, # 56
    KEY_9, # 57
    0, # 58
    0, # 59
    0, # 60
    0, # 61
    0, # 62
    0, # 63
    0, # 64
    KEY_A, # 65
    KEY_B, # 66
    KEY_C, # 67
    KEY_D, # 68
    KEY_E, # 69
    KEY_F, # 70
    KEY_G, # 71
    KEY_H, # 72
    KEY_I, # 73
    KEY_J, # 74
    KEY_K, # 75
    KEY_L, # 76
    KEY_M, # 77
    KEY_N, # 78
    KEY_O, # 79
    KEY_P, # 80
    KEY_Q, # 81
    KEY_R, # 82
    KEY_S, # 83
    KEY_T, # 84
    KEY_U, # 85
    KEY_V, # 86
    KEY_W, # 87
    KEY_X, # 88
    KEY_Y, # 89
    KEY_Z, # 90
    0, # 91
    0, # 92
    0, # 93
    0, # 94
    0, # 95
    0, # 96
    KEY_A, # 97
    KEY_B, # 98
    KEY_C, # 99
    KEY_D, # 100
    KEY_E, # 101
    KEY_F, # 102
    KEY_G, # 103
    KEY_H, # 104
    KEY_I, # 105
    KEY_J, # 106
    KEY_K, # 107
    KEY_L, # 108
    KEY_M, # 109
    KEY_N, # 110
    KEY_O, # 111
    KEY_P, # 112
    KEY_Q, # 113
    KEY_R, # 114
    KEY_S, # 115
    KEY_T, # 116
    KEY_U, # 117
    KEY_V, # 118
    KEY_W, # 119
    KEY_X, # 120
    KEY_Y, # 121
    KEY_Z, # 122
    0, # 123
    0, # 124
    0, # 125
    0, # 126
    0, # 127
    0, # 128
    0, # 129
    0, # 130
    0, # 131
    0, # 132
    0, # 133
    0, # 134
    0, # 135
    0, # 136
    0, # 137
    0, # 138
    0, # 139
    0, # 140
    0, # 141
    0, # 142
    0, # 143
    0, # 144
    0, # 145
    0, # 146
    0, # 147
    0, # 148
    0, # 149
    0, # 150
    0, # 151
    0, # 152
    0, # 153
    0, # 154
    0, # 155
    0, # 156
    0, # 157
    0, # 158
    0, # 159
    0, # 160
    0, # 161
    0, # 162
    0, # 163
    0, # 164
    0, # 165
    0, # 166
    0, # 167
    0, # 168
    0, # 169
    0, # 170
    0, # 171
    0, # 172
    0, # 173
    0, # 174
    0, # 175
    0, # 176
    0, # 177
    0, # 178
    0, # 179
    0, # 180
    0, # 181
    0, # 182
    0, # 183
    0, # 184
    0, # 185
    0, # 186
    0, # 187
    0, # 188
    0, # 189
    0, # 190
    0, # 191
    0, # 192
    0, # 193
    0, # 194
    0, # 195
    0, # 196
    0, # 197
    0, # 198
    0, # 199
    0, # 200
    0, # 201
    0, # 202
    0, # 203
    0, # 204
    0, # 205
    0, # 206
    0, # 207
    0, # 208
    0, # 209
    0, # 210
    0, # 211
    0, # 212
    0, # 213
    0, # 214
    0, # 215
    0, # 216
    0, # 217
    0, # 218
    0, # 219
    0, # 220
    0, # 221
    0, # 222
    0, # 223
    0, # 224
    0, # 225
    0, # 226
    0, # 227
    0, # 228
    0, # 229
    0, # 230
    0, # 231
    0, # 232
    0, # 233
    0, # 234
    0, # 235
    0, # 236
    0, # 237
    0, # 238
    0, # 239
    0, # 240
    0, # 241
    0, # 242
    0, # 243
    0, # 244
    0, # 245
    0, # 246
    0, # 247
    0, # 248
    0, # 249
    0, # 250
    0, # 251
    0, # 252
    0, # 253
    0, # 254
    0, # 255
]

def send_gps_message(f, msg):
    f.write(msg + '\n')
    f.flush()


def send(host, port, text):
    s = socket()
    s.connect((host, port))
    f = s.makefile(mode='w')
    #f.write(make_time_msg() + '\n')
    sleep(0.5)
    for k in text:
        send_gps_message(f, make_scan_msg(key_to_scan[ord(k)]))
    for i in range(3):
        send_gps_message(f, make_scan_msg(KEY_UP))

if __name__ == '__main__':
    send(host='localhost', port=10110, text='hello\n1234\ngoodbye\n')
    #for x in sys.argv[1:]:
    #    print(f'{x} => {make_msg(x)}')
