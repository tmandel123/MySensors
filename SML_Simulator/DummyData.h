// clang-format off
// Formatted example data from https://github.com/devZer0/libsml-testing
const unsigned int HLY_bin_len = 560;

const unsigned char HLY_bin1[] = {
0x1b,0x1b,0x1b,0x1b,0x01,0x01,0x01,0x01,0x76,0x04,0x00,0x00,0x01,0x62,0x00,0x62,
0x00,0x72,0x65,0x00,0x00,0x01,0x01,0x76,0x01,0x01,0x07,0x00,0x00,0x00,0x12,0xd6,
0x8b,0x0b,0x0a,0x01,0x48,0x4c,0x59,0x02,0x00,0x15,0xb8,0xaa,0x72,0x62,0x01,0x65,
0x00,0x12,0xd6,0x8b,0x01,0x63,0xda,0xe4,0x00,0x76,0x04,0x00,0x00,0x02,0x62,0x00,
0x62,0x00,0x72,0x65,0x00,0x00,0x07,0x01,0x77,0x07,0xff,0xff,0xff,0xff,0xff,0xff,
0x0b,0x0a,0x01,0x48,0x4c,0x59,0x02,0x00,0x15,0xb8,0xaa,0x07,0x01,0x00,0x62,0x0a,
0xff,0xff,0x72,0x62,0x01,0x65,0x00,0x12,0xd6,0x8b,0xf1,0x06,0x77,0x07,0x01,0x00,
0x60,0x32,0x01,0x01,0x01,0x01,0x01,0x01,0x04,0x48,0x4c,0x59,0x01,0x77,0x07,0x01,
0x00,0x60,0x01,0x00,0xff,0x01,0x01,0x01,0x01,0x0b,0x0a,0x01,0x48,0x4c,0x59,0x02,
0x00,0x15,0xb8,0xaa,0x01,0x77,0x07,0x01,0x00,0x01,0x08,0x00,0xff,0x65,0x00,0x1c,
0x81,0x04,0x72,0x62,0x01,0x65,0x00,0x12,0xd6,0x8b,0x62,0x1e,0x52,0xff,0x65,0x00,
0x14,0x29,0x18,0x01,0x77,0x07,0x01,0x00,0x02,0x08,0x00,0xff,0x01,0x72,0x62,0x01,
0x65,0x00,0x12,0xd6,0x8b,0x62,0x1e,0x52,0xff,0x63,0x07,0x35,0x01,0x77,0x07,0x01,
0x00,0x10,0x07,0x00,0xff,0x01,0x01,0x62,0x1b,0x52,0x00,0x53,0x00,0xcc,0x01,0x77,
0x07,0x01,0x00,0x24,0x07,0x00,0xff,0x01,0x01,0x62,0x1b,0x52,0x00,0x52,0x41,0x01,
0x77,0x07,0x01,0x00,0x38,0x07,0x00,0xff,0x01,0x01,0x62,0x1b,0x52,0x00,0x52,0x58,
0x01,0x77,0x07,0x01,0x00,0x4c,0x07,0x00,0xff,0x01,0x01,0x62,0x1b,0x52,0x00,0x52,
0x31,0x01,0x77,0x07,0x01,0x00,0x20,0x07,0x00,0xff,0x01,0x01,0x62,0x23,0x52,0xff,
0x63,0x09,0x49,0x01,0x77,0x07,0x01,0x00,0x34,0x07,0x00,0xff,0x01,0x01,0x62,0x23,
0x52,0xff,0x63,0x09,0x42,0x01,0x77,0x07,0x01,0x00,0x48,0x07,0x00,0xff,0x01,0x01,
0x62,0x23,0x52,0xff,0x63,0x09,0x50,0x01,0x77,0x07,0x01,0x00,0x1f,0x07,0x00,0xff,
0x01,0x01,0x62,0x21,0x52,0xfe,0x62,0x37,0x01,0x77,0x07,0x01,0x00,0x33,0x07,0x00,
0xff,0x01,0x01,0x62,0x21,0x52,0xfe,0x62,0x33,0x01,0x77,0x07,0x01,0x00,0x47,0x07,
0x00,0xff,0x01,0x01,0x62,0x21,0x52,0xfe,0x62,0x25,0x01,0x77,0x07,0x01,0x00,0x51,
0x07,0x01,0xff,0x01,0x01,0x62,0x08,0x52,0x00,0x53,0x00,0xf1,0x01,0x77,0x07,0x01,
0x00,0x51,0x07,0x02,0xff,0x01,0x01,0x62,0x08,0x52,0x00,0x52,0x78,0x01,0x77,0x07,
0x01,0x00,0x51,0x07,0x04,0xff,0x01,0x01,0x62,0x08,0x52,0x00,0x53,0x01,0x2b,0x01,
0x77,0x07,0x01,0x00,0x51,0x07,0x0f,0xff,0x01,0x01,0x62,0x08,0x52,0x00,0x53,0x01,
0x3c,0x01,0x77,0x07,0x01,0x00,0x51,0x07,0x1a,0xff,0x01,0x01,0x62,0x08,0x52,0x00,
0x53,0x01,0x2f,0x01,0x77,0x07,0x01,0x00,0x0e,0x07,0x00,0xff,0x01,0x01,0x62,0x2c,
0x52,0xff,0x63,0x01,0xf4,0x01,0x77,0x07,0x01,0x00,0x00,0x02,0x00,0x00,0x01,0x01,
0x01,0x01,0x07,0x35,0x30,0x31,0x30,0x30,0x32,0x01,0x77,0x07,0x01,0x00,0x60,0x5a,
0x02,0x01,0x01,0x01,0x01,0x01,0x05,0x36,0x31,0x33,0x39,0x01,0x01,0x01,0x63,0x53,
0x87,0x00,0x76,0x04,0x00,0x00,0x03,0x62,0x00,0x62,0x00,0x72,0x65,0x00,0x00,0x02,
0x01,0x71,0x01,0x63,0xe8,0x23,0x00,0x00,0x1b,0x1b,0x1b,0x1b,0x1a,0x01,0xa3,0xd2
};

const unsigned char HLY_bin2[] = {
0x1b,0x1b,0x1b,0x1b,0x01,0x01,0x01,0x01,0x76,0x04,0x00,0x00,0x01,0x62,0x00,0x62,
0x00,0x72,0x65,0x00,0x00,0x01,0x01,0x76,0x01,0x01,0x07,0x00,0x00,0x00,0x12,0xd6,
0x8e,0x0b,0x0a,0x01,0x48,0x4c,0x59,0x02,0x00,0x15,0xb8,0xaa,0x72,0x62,0x01,0x65,
0x00,0x12,0xd6,0x8e,0x01,0x63,0xbe,0x70,0x00,0x76,0x04,0x00,0x00,0x02,0x62,0x00,
0x62,0x00,0x72,0x65,0x00,0x00,0x07,0x01,0x77,0x07,0xff,0xff,0xff,0xff,0xff,0xff,
0x0b,0x0a,0x01,0x48,0x4c,0x59,0x02,0x00,0x15,0xb8,0xaa,0x07,0x01,0x00,0x62,0x0a,
0xff,0xff,0x72,0x62,0x01,0x65,0x00,0x12,0xd6,0x8e,0xf1,0x06,0x77,0x07,0x01,0x00,
0x60,0x32,0x01,0x01,0x01,0x01,0x01,0x01,0x04,0x48,0x4c,0x59,0x01,0x77,0x07,0x01,
0x00,0x60,0x01,0x00,0xff,0x01,0x01,0x01,0x01,0x0b,0x0a,0x01,0x48,0x4c,0x59,0x02,
0x00,0x15,0xb8,0xaa,0x01,0x77,0x07,0x01,0x00,0x01,0x08,0x00,0xff,0x65,0x00,0x1c,
0x81,0x04,0x72,0x62,0x01,0x65,0x00,0x12,0xd6,0x8e,0x62,0x1e,0x52,0xff,0x65,0x00,
0x14,0x29,0x19,0x01,0x77,0x07,0x01,0x00,0x02,0x08,0x00,0xff,0x01,0x72,0x62,0x01,
0x65,0x00,0x12,0xd6,0x8e,0x62,0x1e,0x52,0xff,0x63,0x07,0x35,0x01,0x77,0x07,0x01,
0x00,0x10,0x07,0x00,0xff,0x01,0x01,0x62,0x1b,0x52,0x00,0x53,0x00,0xd3,0x01,0x77,
0x07,0x01,0x00,0x24,0x07,0x00,0xff,0x01,0x01,0x62,0x1b,0x52,0x00,0x52,0x48,0x01,
0x77,0x07,0x01,0x00,0x38,0x07,0x00,0xff,0x01,0x01,0x62,0x1b,0x52,0x00,0x52,0x59,
0x01,0x77,0x07,0x01,0x00,0x4c,0x07,0x00,0xff,0x01,0x01,0x62,0x1b,0x52,0x00,0x52,
0x31,0x01,0x77,0x07,0x01,0x00,0x20,0x07,0x00,0xff,0x01,0x01,0x62,0x23,0x52,0xff,
0x63,0x09,0x42,0x01,0x77,0x07,0x01,0x00,0x34,0x07,0x00,0xff,0x01,0x01,0x62,0x23,
0x52,0xff,0x63,0x09,0x47,0x01,0x77,0x07,0x01,0x00,0x48,0x07,0x00,0xff,0x01,0x01,
0x62,0x23,0x52,0xff,0x63,0x09,0x50,0x01,0x77,0x07,0x01,0x00,0x1f,0x07,0x00,0xff,
0x01,0x01,0x62,0x21,0x52,0xfe,0x62,0x3b,0x01,0x77,0x07,0x01,0x00,0x33,0x07,0x00,
0xff,0x01,0x01,0x62,0x21,0x52,0xfe,0x62,0x33,0x01,0x77,0x07,0x01,0x00,0x47,0x07,
0x00,0xff,0x01,0x01,0x62,0x21,0x52,0xfe,0x62,0x25,0x01,0x77,0x07,0x01,0x00,0x51,
0x07,0x01,0xff,0x01,0x01,0x62,0x08,0x52,0x00,0x53,0x00,0xf0,0x01,0x77,0x07,0x01,
0x00,0x51,0x07,0x02,0xff,0x01,0x01,0x62,0x08,0x52,0x00,0x52,0x78,0x01,0x77,0x07,
0x01,0x00,0x51,0x07,0x04,0xff,0x01,0x01,0x62,0x08,0x52,0x00,0x53,0x01,0x2d,0x01,
0x77,0x07,0x01,0x00,0x51,0x07,0x0f,0xff,0x01,0x01,0x62,0x08,0x52,0x00,0x53,0x01,
0x3d,0x01,0x77,0x07,0x01,0x00,0x51,0x07,0x1a,0xff,0x01,0x01,0x62,0x08,0x52,0x00,
0x53,0x01,0x2e,0x01,0x77,0x07,0x01,0x00,0x0e,0x07,0x00,0xff,0x01,0x01,0x62,0x2c,
0x52,0xff,0x63,0x01,0xf4,0x01,0x77,0x07,0x01,0x00,0x00,0x02,0x00,0x00,0x01,0x01,
0x01,0x01,0x07,0x35,0x30,0x31,0x30,0x30,0x32,0x01,0x77,0x07,0x01,0x00,0x60,0x5a,
0x02,0x01,0x01,0x01,0x01,0x01,0x05,0x36,0x31,0x33,0x39,0x01,0x01,0x01,0x63,0x06,
0x2f,0x00,0x76,0x04,0x00,0x00,0x03,0x62,0x00,0x62,0x00,0x72,0x65,0x00,0x00,0x02,
0x01,0x71,0x01,0x63,0xe8,0x23,0x00,0x00,0x1b,0x1b,0x1b,0x1b,0x1a,0x01,0xcf,0xf9
};

const unsigned char HLY_bin3[] = {
0x1b,0x1b,0x1b,0x1b,0x01,0x01,0x01,0x01,0x76,0x04,0x00,0x00,0x01,0x62,0x00,0x62,
0x00,0x72,0x65,0x00,0x00,0x01,0x01,0x76,0x01,0x01,0x07,0x00,0x00,0x00,0x12,0xd6,
0x90,0x0b,0x0a,0x01,0x48,0x4c,0x59,0x02,0x00,0x15,0xb8,0xaa,0x72,0x62,0x01,0x65,
0x00,0x12,0xd6,0x90,0x01,0x63,0xd5,0x11,0x00,0x76,0x04,0x00,0x00,0x02,0x62,0x00,
0x62,0x00,0x72,0x65,0x00,0x00,0x07,0x01,0x77,0x07,0xff,0xff,0xff,0xff,0xff,0xff,
0x0b,0x0a,0x01,0x48,0x4c,0x59,0x02,0x00,0x15,0xb8,0xaa,0x07,0x01,0x00,0x62,0x0a,
0xff,0xff,0x72,0x62,0x01,0x65,0x00,0x12,0xd6,0x90,0xf1,0x06,0x77,0x07,0x01,0x00,
0x60,0x32,0x01,0x01,0x01,0x01,0x01,0x01,0x04,0x48,0x4c,0x59,0x01,0x77,0x07,0x01,
0x00,0x60,0x01,0x00,0xff,0x01,0x01,0x01,0x01,0x0b,0x0a,0x01,0x48,0x4c,0x59,0x02,
0x00,0x15,0xb8,0xaa,0x01,0x77,0x07,0x01,0x00,0x01,0x08,0x00,0xff,0x65,0x00,0x1c,
0x81,0x04,0x72,0x62,0x01,0x65,0x00,0x12,0xd6,0x90,0x62,0x1e,0x52,0xff,0x65,0x00,
0x14,0x29,0x1b,0x01,0x77,0x07,0x01,0x00,0x02,0x08,0x00,0xff,0x01,0x72,0x62,0x01,
0x65,0x00,0x12,0xd6,0x90,0x62,0x1e,0x52,0xff,0x63,0x07,0x35,0x01,0x77,0x07,0x01,
0x00,0x10,0x07,0x00,0xff,0x01,0x01,0x62,0x1b,0x52,0x00,0x53,0x00,0xcb,0x01,0x77,
0x07,0x01,0x00,0x24,0x07,0x00,0xff,0x01,0x01,0x62,0x1b,0x52,0x00,0x52,0x41,0x01,
0x77,0x07,0x01,0x00,0x38,0x07,0x00,0xff,0x01,0x01,0x62,0x1b,0x52,0x00,0x52,0x59,
0x01,0x77,0x07,0x01,0x00,0x4c,0x07,0x00,0xff,0x01,0x01,0x62,0x1b,0x52,0x00,0x52,
0x30,0x01,0x77,0x07,0x01,0x00,0x20,0x07,0x00,0xff,0x01,0x01,0x62,0x23,0x52,0xff,
0x63,0x09,0x41,0x01,0x77,0x07,0x01,0x00,0x34,0x07,0x00,0xff,0x01,0x01,0x62,0x23,
0x52,0xff,0x63,0x09,0x47,0x01,0x77,0x07,0x01,0x00,0x48,0x07,0x00,0xff,0x01,0x01,
0x62,0x23,0x52,0xff,0x63,0x09,0x4f,0x01,0x77,0x07,0x01,0x00,0x1f,0x07,0x00,0xff,
0x01,0x01,0x62,0x21,0x52,0xfe,0x62,0x36,0x01,0x77,0x07,0x01,0x00,0x33,0x07,0x00,
0xff,0x01,0x01,0x62,0x21,0x52,0xfe,0x62,0x33,0x01,0x77,0x07,0x01,0x00,0x47,0x07,
0x00,0xff,0x01,0x01,0x62,0x21,0x52,0xfe,0x62,0x25,0x01,0x77,0x07,0x01,0x00,0x51,
0x07,0x01,0xff,0x01,0x01,0x62,0x08,0x52,0x00,0x53,0x00,0xf0,0x01,0x77,0x07,0x01,
0x00,0x51,0x07,0x02,0xff,0x01,0x01,0x62,0x08,0x52,0x00,0x52,0x76,0x01,0x77,0x07,
0x01,0x00,0x51,0x07,0x04,0xff,0x01,0x01,0x62,0x08,0x52,0x00,0x53,0x01,0x2c,0x01,
0x77,0x07,0x01,0x00,0x51,0x07,0x0f,0xff,0x01,0x01,0x62,0x08,0x52,0x00,0x53,0x01,
0x3d,0x01,0x77,0x07,0x01,0x00,0x51,0x07,0x1a,0xff,0x01,0x01,0x62,0x08,0x52,0x00,
0x53,0x01,0x2e,0x01,0x77,0x07,0x01,0x00,0x0e,0x07,0x00,0xff,0x01,0x01,0x62,0x2c,
0x52,0xff,0x63,0x01,0xf4,0x01,0x77,0x07,0x01,0x00,0x00,0x02,0x00,0x00,0x01,0x01,
0x01,0x01,0x07,0x35,0x30,0x31,0x30,0x30,0x32,0x01,0x77,0x07,0x01,0x00,0x60,0x5a,
0x02,0x01,0x01,0x01,0x01,0x01,0x05,0x36,0x31,0x33,0x39,0x01,0x01,0x01,0x63,0x13,
0xfa,0x00,0x76,0x04,0x00,0x00,0x03,0x62,0x00,0x62,0x00,0x72,0x65,0x00,0x00,0x02,
0x01,0x71,0x01,0x63,0xe8,0x23,0x00,0x00,0x1b,0x1b,0x1b,0x1b,0x1a,0x01,0xcd,0x20
};

const unsigned char HLY_Einspeisung[] = {
0x1b,0x1b,0x1b,0x1b,0x01,0x01,0x01,0x01,0x76,0x04,0x00,0x00,0x01,0x62,0x00,0x62,
0x00,0x72,0x65,0x00,0x00,0x01,0x01,0x76,0x01,0x01,0x07,0x00,0x00,0x00,0x12,0x9c,
0x06,0x0b,0x0a,0x01,0x48,0x4c,0x59,0x02,0x00,0x15,0xb8,0xaa,0x72,0x62,0x01,0x65,
0x00,0x12,0x9c,0x06,0x01,0x63,0xa9,0xae,0x00,0x76,0x04,0x00,0x00,0x02,0x62,0x00,
0x62,0x00,0x72,0x65,0x00,0x00,0x07,0x01,0x77,0x07,0xff,0xff,0xff,0xff,0xff,0xff,
0x0b,0x0a,0x01,0x48,0x4c,0x59,0x02,0x00,0x15,0xb8,0xaa,0x07,0x01,0x00,0x62,0x0a,
0xff,0xff,0x72,0x62,0x01,0x65,0x00,0x12,0x9c,0x06,0xf1,0x06,0x77,0x07,0x01,0x00,
0x60,0x32,0x01,0x01,0x01,0x01,0x01,0x01,0x04,0x48,0x4c,0x59,0x01,0x77,0x07,0x01,
0x00,0x60,0x01,0x00,0xff,0x01,0x01,0x01,0x01,0x0b,0x0a,0x01,0x48,0x4c,0x59,0x02,
0x00,0x15,0xb8,0xaa,0x01,0x77,0x07,0x01,0x00,0x01,0x08,0x00,0xff,0x65,0x00,0x1c,
0x91,0x04,0x72,0x62,0x01,0x65,0x00,0x12,0x9c,0x06,0x62,0x1e,0x52,0xff,0x65,0x00,
0x13,0xe9,0x89,0x01,0x77,0x07,0x01,0x00,0x02,0x08,0x00,0xff,0x01,0x72,0x62,0x01,
0x65,0x00,0x12,0x9c,0x06,0x62,0x1e,0x52,0xff,0x63,0x06,0x3b,0x01,0x77,0x07,0x01,
0x00,0x10,0x07,0x00,0xff,0x01,0x01,0x62,0x1b,0x52,0x00,0x52,0x22,0x01,0x77,0x07,
0x01,0x00,0x24,0x07,0x00,0xff,0x01,0x01,0x62,0x1b,0x52,0x00,0x52,0x89,0x01,0x77,
0x07,0x01,0x00,0x38,0x07,0x00,0xff,0x01,0x01,0x62,0x1b,0x52,0x00,0x52,0x3e,0x01,
0x77,0x07,0x01,0x00,0x4c,0x07,0x00,0xff,0x01,0x01,0x62,0x1b,0x52,0x00,0x52,0x5b,
0x01,0x77,0x07,0x01,0x00,0x20,0x07,0x00,0xff,0x01,0x01,0x62,0x23,0x52,0xff,0x63,
0x09,0x3d,0x01,0x77,0x07,0x01,0x00,0x34,0x07,0x00,0xff,0x01,0x01,0x62,0x23,0x52,
0xff,0x63,0x09,0x46,0x01,0x77,0x07,0x01,0x00,0x48,0x07,0x00,0xff,0x01,0x01,0x62,
0x23,0x52,0xff,0x63,0x09,0x40,0x01,0x77,0x07,0x01,0x00,0x1f,0x07,0x00,0xff,0x01,
0x01,0x62,0x21,0x52,0xfe,0x62,0x45,0x01,0x77,0x07,0x01,0x00,0x33,0x07,0x00,0xff,
0x01,0x01,0x62,0x21,0x52,0xfe,0x62,0x30,0x01,0x77,0x07,0x01,0x00,0x47,0x07,0x00,
0xff,0x01,0x01,0x62,0x21,0x52,0xfe,0x62,0x49,0x01,0x77,0x07,0x01,0x00,0x51,0x07,
0x01,0xff,0x01,0x01,0x62,0x08,0x52,0x00,0x53,0x00,0xf0,0x01,0x77,0x07,0x01,0x00,
0x51,0x07,0x02,0xff,0x01,0x01,0x62,0x08,0x52,0x00,0x52,0x78,0x01,0x77,0x07,0x01,
0x00,0x51,0x07,0x04,0xff,0x01,0x01,0x62,0x08,0x52,0x00,0x53,0x00,0xdf,0x01,0x77,
0x07,0x01,0x00,0x51,0x07,0x0f,0xff,0x01,0x01,0x62,0x08,0x52,0x00,0x53,0x01,0x2f,
0x01,0x77,0x07,0x01,0x00,0x51,0x07,0x1a,0xff,0x01,0x01,0x62,0x08,0x52,0x00,0x53,
0x01,0x2d,0x01,0x77,0x07,0x01,0x00,0x0e,0x07,0x00,0xff,0x01,0x01,0x62,0x2c,0x52,
0xff,0x63,0x01,0xf4,0x01,0x77,0x07,0x01,0x00,0x00,0x02,0x00,0x00,0x01,0x01,0x01,
0x01,0x07,0x35,0x30,0x31,0x30,0x30,0x32,0x01,0x77,0x07,0x01,0x00,0x60,0x5a,0x02,
0x01,0x01,0x01,0x01,0x01,0x05,0x36,0x31,0x33,0x39,0x01,0x01,0x01,0x63,0x8b,0xb3,
0x00,0x76,0x04,0x00,0x00,0x03,0x62,0x00,0x62,0x00,0x72,0x65,0x00,0x00,0x02,0x01,
0x71,0x01,0x63,0xe8,0x23,0x00,0x00,0x00,0x1b,0x1b,0x1b,0x1b,0x1a,0x02,0xed,0xd4
};
