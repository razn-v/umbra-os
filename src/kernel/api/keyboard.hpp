// I don't like this
#ifdef KERNEL
namespace Keyboard {
#endif

#define KEYS \
    KEY(Invalid, 0x0, '?', '?') \
    KEY(Escape, 0x1, '?', '?') \
    KEY(Num1, 0x2, '1', '!') \
    KEY(Num2, 0x3, '2', '@') \
    KEY(Num3, 0x4, '3', '#') \
    KEY(Num4, 0x5, '4', '$') \
    KEY(Num5, 0x6, '5', '%') \
    KEY(Num6, 0x7, '6', '^') \
    KEY(Num7, 0x8, '7', '&') \
    KEY(Num8, 0x9, '8', '*') \
    KEY(Num9, 0xa, '9', '(') \
    KEY(Num0, 0xb, '0', ')') \
    KEY(Minus, 0xc, '-', '?') \
    KEY(Equals, 0xd, '=', '?') \
    KEY(BackSpace, 0xe, '?', '?') \
    KEY(Tab, 0xf, '?', '?') \
    KEY(Q, 0x10, 'q', 'Q') \
    KEY(W, 0x11, 'w', 'W') \
    KEY(E, 0x12, 'e', 'E') \
    KEY(R, 0x13, 'r', 'R') \
    KEY(T, 0x14, 't', 'T') \
    KEY(Y, 0x15, 'y', 'Y') \
    KEY(U, 0x16, 'u', 'U') \
    KEY(I, 0x17, 'i', 'I') \
    KEY(O, 0x18, 'o', 'O') \
    KEY(P, 0x19, 'p', 'P') \
    KEY(OpenBracket, 0x1a, '[', '?') \
    KEY(CloseBracket, 0x1b, ']', '?') \
    KEY(Enter, 0x1c, '\n', '\n') \
    KEY(LeftControl, 0x1d, '?', '?') \
    KEY(A, 0x1e, 'a', 'A') \
    KEY(S, 0x1f, 's', 'S') \
    KEY(D, 0x20, 'd', 'D') \
    KEY(F, 0x21, 'f', 'F') \
    KEY(G, 0x22, 'g', 'G') \
    KEY(H, 0x23, 'h', 'H') \
    KEY(J, 0x24, 'j', 'J') \
    KEY(K, 0x25, 'k', 'K') \
    KEY(L, 0x26, 'l', 'L') \
    KEY(Semicolon, 0x27, ';', '?') \
    KEY(SingleQuote, 0x28, '\'', '?') \
    KEY(BackTick, 0x29, '`', '?') \
    KEY(LeftShift, 0x2a, '?', '?') \
    KEY(BackSlash, 0x2b, '\\', '?') \
    KEY(Z, 0x2c, 'z', 'Z') \
    KEY(X, 0x2d, 'x', 'X') \
    KEY(C, 0x2e, 'c', 'C') \
    KEY(V, 0x2f, 'v', 'V') \
    KEY(B, 0x30, 'b', 'B') \
    KEY(N, 0x31, 'n', 'N') \
    KEY(M, 0x32, 'm', 'M') \
    KEY(Comma, 0x33, ',', '?') \
    KEY(Dot, 0x34, '.', '?') \
    KEY(Slash, 0x35, '/', '?') \
    KEY(RightShift, 0x36, '?', '?') \
    KEY(Multiply, 0x37, '*', '?') \
    KEY(LeftAlt, 0x38, '?', '?') \
    KEY(Space, 0x39, ' ', ' ') \
    KEY(CapsLock, 0x3a, '?', '?') \
    KEY(F1, 0x3b, '?', '?') \
    KEY(F2, 0x3c, '?', '?') \
    KEY(F3, 0x3d, '?', '?') \
    KEY(F4, 0x3e, '?', '?') \
    KEY(F5, 0x3f, '?', '?') \
    KEY(F6, 0x40, '?', '?') \
    KEY(F7, 0x41, '?', '?') \
    KEY(F8, 0x42, '?', '?') \
    KEY(F9, 0x43, '?', '?') \
    KEY(F10, 0x44, '?', '?') \
    KEY(NumberLock, 0x45, '?', '?') \
    KEY(ScrollLock, 0x46, '?', '?') \
    KEY(NumPad7, 0x47, '7', '7') \
    KEY(NumPad8, 0x48, '8', '8') \
    KEY(NumPad9, 0x49, '9', '9') \
    KEY(NumPadMinus, 0x4a, '-', '-') \
    KEY(NumPad4, 0x4b, '4', '4') \
    KEY(NumPad5, 0x4c, '5', '5') \
    KEY(NumPad6, 0x4d, '6', '6') \
    KEY(NumPadPlus, 0x4e, '+', '+') \
    KEY(NumPad1, 0x4f, '1', '1') \
    KEY(NumPad2, 0x50, '2', '2') \
    KEY(NumPad3, 0x51, '3', '3') \
    KEY(NumPad0, 0x52, '0', '0') \
    KEY(NumPadDot, 0x53, '.', '.') \
    KEY(F11, 0x57, '?', '?') \
    KEY(F12, 0x58, '?', '?') \

#define KEY(id, scancode, ascii, uppercase_ascii) id = scancode,
enum class KeyCode {
    KEYS
};
#undef KEY

struct KeyboardEvent {
    KeyCode key_code;
    char ascii;

    constexpr KeyboardEvent() = default;
    KeyboardEvent(KeyCode code, char ascii) : key_code(code), ascii(ascii) {};
};

#ifdef KERNEL
}
#endif
