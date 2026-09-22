#include <cassert>
#include <cstddef>
#include <string>

static char unescape(const char c) {
  switch (c) {
    case '\\':
      return '\\';
    case 'n':
      return '\n';
    case '"':
      return '"';
    default:
      return c;
  }
}

std::string unescape(const std::string& s) {
  std::string ans;
  ans.reserve(s.size());

  assert(s[0] == '"' && "start is '\"' ");
  assert(s[s.size() - 1] == '"' && "end is '\"' ");

  for (size_t i = 1; i < s.size() - 1; i++) {
    if (s[i] == '\\') {
      i++;
      if (i != s.size()) {
        ans += unescape(s[i]);
      }
    } else {
      ans += s[i];
    }
  }
  ans.shrink_to_fit();
  return ans;
}

std::string escape(const std::string& s) {
  std::string ans;
  ans.reserve(s.size() * 2 + 2);
  ans += '"';
  for (size_t i = 0; i < s.size(); i++) {
    char c = s[i];
    switch (c) {
      case '\n':
        ans += "\\n";
        break;
      case '\\':
        ans += "\\\\";
        break;
      case '"':
        ans += "\\\"";
        break;
      default:
        ans += c;
    }
  }
  ans += '"';
  ans.shrink_to_fit();
  return ans;
}
