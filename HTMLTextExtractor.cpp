#include <windows.h>
#include <string>
#include <regex>
#include "PluginInterface.h"

NppData nppData;
FuncItem funcItem[1];

HWND getCurrentScintilla() {
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1) return NULL;
    return (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
}

void extractHTMLText() {
    HWND curScintilla = getCurrentScintilla();
    if (!curScintilla) return;

    size_t length = ::SendMessage(curScintilla, SCI_GETLENGTH, 0, 0);
    if (length == 0) return;

    char* buffer = new char[length + 1];
    ::SendMessage(curScintilla, SCI_GETTEXT, length + 1, (LPARAM)buffer);
    std::string text(buffer);
    delete[] buffer;

    // 1. Remove <script> and <style> blocks completely
    text = std::regex_replace(text, std::regex(R"(<script[\s\S]*?</script>)", std::regex::icase), "\n");
    text = std::regex_replace(text, std::regex(R"(<style[\s\S]*?</style>)", std::regex::icase), "\n");

    // 2. Convert <br> to newline
    text = std::regex_replace(text, std::regex(R"(<br\s*/?>)", std::regex::icase), "\n");

    // 3. Add newlines for structural / block elements (this is the key improvement)
    //     - Headings, paragraphs, divs, sections → double newline (new paragraph)
    //     - Buttons → single newline (each option on its own line)
    text = std::regex_replace(text, std::regex(R"(</(h[1-6]|p|div|section|article|header|footer|nav)>)", std::regex::icase), "\n\n");
    text = std::regex_replace(text, std::regex(R"(</button>)", std::regex::icase), "\n");
    text = std::regex_replace(text, std::regex(R"(<li[^>]*>)", std::regex::icase), "\n• ");

    // 4. Remove ALL remaining HTML tags (replace with single space)
    text = std::regex_replace(text, std::regex(R"(<[^>]+>)"), " ");

    // 5. Normalize whitespace:
    //    - Multiple horizontal spaces → single space
    //    - Multiple newlines → max 2 blank lines
    text = std::regex_replace(text, std::regex(R"([ \t\r\f]+)"), " ");
    text = std::regex_replace(text, std::regex(R"(\n\s*\n)"), "\n\n");

    // 6. Trim leading/trailing whitespace
    size_t startpos = text.find_first_not_of(" \t\n\r\f");
    if (startpos == std::string::npos) {
        text = "";
    } else {
        size_t endpos = text.find_last_not_of(" \t\n\r\f");
        text = text.substr(startpos, endpos - startpos + 1);
    }

    ::SendMessage(curScintilla, SCI_SETTEXT, 0, (LPARAM)text.c_str());
}


extern "C" __declspec(dllexport) void setInfo(NppData notepadPlusData) {
    nppData = notepadPlusData;
}

extern "C" __declspec(dllexport) const wchar_t* getName() {
    return L"HTMLTextExtractor";
}

extern "C" __declspec(dllexport) FuncItem* getFuncsArray(int* nbF) {
    funcItem[0]._pFunc = extractHTMLText;
    wcscpy(funcItem[0]._itemName, L"Extract Text from HTML");
    funcItem[0]._init2Check = false;
    funcItem[0]._pShKey = NULL;

    *nbF = 1;
    return funcItem;
}

extern "C" __declspec(dllexport) void beNotified(SCNotification* notifyCode) {
    // not needed
}

extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam) {
    return TRUE;
}

extern "C" __declspec(dllexport) BOOL isUnicode() {
    return TRUE;
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD reasonForCall, LPVOID lpReserved) {
    return TRUE;
}
