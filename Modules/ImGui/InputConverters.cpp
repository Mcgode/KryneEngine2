/**
 * @file
 * @author Max Godefroy
 * @date 16/08/2024.
 */

#include "InputConverters.hpp"

namespace KryneEngine::Modules::ImGui::InputConverters
{
    ImGuiKey ToImGuiKey(InputPhysicalKeys _key)
    {
#define MAP(keyName, imguiKey) case InputPhysicalKeys::keyName: return imguiKey

        switch (_key)
        {
        MAP(Unknown, ImGuiKey_None);
        MAP(Space, ImGuiKey_Space);
        MAP(Apostrophe, ImGuiKey_Apostrophe);
        MAP(Comma, ImGuiKey_Comma);
        MAP(Minus, ImGuiKey_Minus);
        MAP(Period, ImGuiKey_Period);
        MAP(Slash, ImGuiKey_Slash);
        MAP(SemiColon, ImGuiKey_Semicolon);
        MAP(Equal, ImGuiKey_Equal);
        MAP(Num0, ImGuiKey_0);
        MAP(Num1, ImGuiKey_1);
        MAP(Num2, ImGuiKey_2);
        MAP(Num3, ImGuiKey_3);
        MAP(Num4, ImGuiKey_4);
        MAP(Num5, ImGuiKey_5);
        MAP(Num6, ImGuiKey_6);
        MAP(Num7, ImGuiKey_7);
        MAP(Num8, ImGuiKey_8);
        MAP(Num9, ImGuiKey_9);
        MAP(A, ImGuiKey_A);
        MAP(B, ImGuiKey_B);
        MAP(C, ImGuiKey_C);
        MAP(D, ImGuiKey_D);
        MAP(E, ImGuiKey_E);
        MAP(F, ImGuiKey_F);
        MAP(G, ImGuiKey_G);
        MAP(H, ImGuiKey_H);
        MAP(I, ImGuiKey_I);
        MAP(J, ImGuiKey_J);
        MAP(K, ImGuiKey_K);
        MAP(L, ImGuiKey_L);
        MAP(M, ImGuiKey_M);
        MAP(N, ImGuiKey_N);
        MAP(O, ImGuiKey_O);
        MAP(P, ImGuiKey_P);
        MAP(Q, ImGuiKey_Q);
        MAP(R, ImGuiKey_R);
        MAP(S, ImGuiKey_S);
        MAP(T, ImGuiKey_T);
        MAP(U, ImGuiKey_U);
        MAP(V, ImGuiKey_V);
        MAP(W, ImGuiKey_W);
        MAP(X, ImGuiKey_X);
        MAP(Y, ImGuiKey_Y);
        MAP(Z, ImGuiKey_Z);
        MAP(LeftBracket, ImGuiKey_LeftBracket);
        MAP(RightBracket, ImGuiKey_RightBracket);
        MAP(BackSlash, ImGuiKey_Backslash);
        MAP(GraveAccent, ImGuiKey_GraveAccent);
        MAP(Escape, ImGuiKey_Escape);
        MAP(Enter, ImGuiKey_Enter);
        MAP(Tab, ImGuiKey_Tab);
        MAP(Backspace, ImGuiKey_Backspace);
        MAP(Insert, ImGuiKey_Insert);
        MAP(Delete, ImGuiKey_Delete);
        MAP(Up, ImGuiKey_UpArrow);
        MAP(Down, ImGuiKey_DownArrow);
        MAP(Right, ImGuiKey_RightArrow);
        MAP(Left, ImGuiKey_LeftArrow);
        MAP(PageUp, ImGuiKey_PageUp);
        MAP(PageDown, ImGuiKey_PageDown);
        MAP(Home, ImGuiKey_Home);
        MAP(End, ImGuiKey_End);
        MAP(CapsLock, ImGuiKey_CapsLock);
        MAP(ScrollLock, ImGuiKey_ScrollLock);
        MAP(NumLock, ImGuiKey_NumLock);
        MAP(PrintScreen, ImGuiKey_PrintScreen);
        MAP(Pause, ImGuiKey_Pause);
        MAP(F1, ImGuiKey_F1);
        MAP(F2, ImGuiKey_F2);
        MAP(F3, ImGuiKey_F3);
        MAP(F4, ImGuiKey_F4);
        MAP(F5, ImGuiKey_F5);
        MAP(F6, ImGuiKey_F6);
        MAP(F7, ImGuiKey_F7);
        MAP(F8, ImGuiKey_F8);
        MAP(F9, ImGuiKey_F9);
        MAP(F10, ImGuiKey_F10);
        MAP(F11, ImGuiKey_F11);
        MAP(F12, ImGuiKey_F12);
        MAP(Keypad0, ImGuiKey_Keypad0);
        MAP(Keypad1, ImGuiKey_Keypad1);
        MAP(Keypad2, ImGuiKey_Keypad2);
        MAP(Keypad3, ImGuiKey_Keypad3);
        MAP(Keypad4, ImGuiKey_Keypad4);
        MAP(Keypad5, ImGuiKey_Keypad5);
        MAP(Keypad6, ImGuiKey_Keypad6);
        MAP(Keypad7, ImGuiKey_Keypad7);
        MAP(Keypad8, ImGuiKey_Keypad8);
        MAP(Keypad9, ImGuiKey_Keypad9);
        MAP(KeypadDecimal, ImGuiKey_KeypadDecimal);
        MAP(KeypadDivide, ImGuiKey_KeypadDivide);
        MAP(KeypadMultiply, ImGuiKey_KeypadMultiply);
        MAP(KeypadSubtract, ImGuiKey_KeypadSubtract);
        MAP(KeypadAdd, ImGuiKey_KeypadAdd);
        MAP(KeypadEnter, ImGuiKey_KeypadEnter);
        MAP(KeypadEqual, ImGuiKey_KeypadEqual);
        MAP(LeftShift, ImGuiKey_LeftShift);
        MAP(LeftCtrl, ImGuiKey_LeftCtrl);
        MAP(LeftAlt, ImGuiKey_LeftAlt);
        MAP(RightShift, ImGuiKey_RightShift);
        MAP(RightCtrl, ImGuiKey_RightCtrl);
        MAP(RightAlt, ImGuiKey_RightAlt);
        MAP(RightSuper, ImGuiKey_RightSuper);
        MAP(Menu, ImGuiKey_Menu);
        }

#undef MAP
    }

    ImGuiMouseButton ToImGuiMouseButton(MouseInputButton _mouseButton)
    {
        switch (_mouseButton)
        {
        case MouseInputButton::Left:
            return ImGuiMouseButton_Left;
        case MouseInputButton::Right:
            return ImGuiMouseButton_Right;
        case MouseInputButton::Middle:
            return ImGuiMouseButton_Middle;
        default:
            return ImGuiMouseButton_COUNT;
        }
    }
} // namespace KryneEngine