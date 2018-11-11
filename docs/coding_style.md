
# Coding style

Darwin's coding style is based on the [Chromium C++ style][1] (which in turn is based on
the [Google C++ style][2]) with a few small tweaks. The notable differences are:

- Function names (including member functions) use camelCase, starting with lowercase
    (ex. createNextGeneration())
- `#pragma once` is used instead of the traditional `#include` guards.
- Although `using directives` are generally [discouraged][3], an exception is made for
    `using namespace std` (even in headers!)
- Max line length is 90 characters.

Also, Darwin adoption of Chromium style is mostly limited to code formatting. The Chromium
and Google styles go beyond formatting and offer C++ language guidance and restrictions. 
While these guidelines have solid rationales and are proven on large code bases, in Darwin
we don’t treat them as the letter of the law, but rather advice.

In particular, Darwin style does not restrict the use of any language feature, or limits
to a particular C++ standard version, although the code must compile on all the supported
platforms and toolchains.

If in doubt regarding formatting, use clang-format (either from the command line or
directly from Qt Creator)

### Tips

- Set your code editor to use Tab & Indent Size = 2, Always use spaces.
- In Qt Creator, under `Options / Beautifier / Clang Format`, Select
    `Use predefined style = File`
- `code_style.xml` provides the Qt Creator style definition (`Options / C++, Import…`)

[1]: https://chromium.googlesource.com/chromium/src/+/master/styleguide/c++/c++.md
[2]: https://google.github.io/styleguide/cppguide.html
[3]: https://abseil.io/tips/153
