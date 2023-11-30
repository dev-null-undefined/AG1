struct TextEditorBackend {
    void assertIndex(size_t i, size_t max) const {
        if (i > max) {
            throw std::out_of_range("Index out of range " + std::to_string(i) + " maximum is " +
                                    std::to_string(max - 1));
        }
    }

    void assertIndex(size_t i) const {
        assertIndex(i, size());
    }

    void assertStrictIndex(size_t i, size_t max) const {
        if (i >= max) {
            throw std::out_of_range("Index out of range " + std::to_string(i) + " maximum is " +
                                    std::to_string(size() - 1));
        }
    }

    void assertStrictIndex(size_t i) const {
        assertStrictIndex(i, size());
    }

    TextEditorBackend(const std::string &text) {
        s = text;
    }

    size_t size() const {
        return s.size();
    }

    size_t lines() const {
        size_t lines = 1;
        for (char c: s) {
            if (c == '\n') {
                ++lines;
            }
        }
        return lines;
    }

    char at(size_t i) const {
        assertStrictIndex(i);
        return s[i];
    }

    void edit(size_t i, char c) {
        assertStrictIndex(i);
        s[i] = c;
    }

    void insert(size_t i, char c) {
        assertIndex(i);
        s.insert(i, 1, c);
    }

    void erase(size_t i) {
        assertStrictIndex(i);
        s.erase(i, 1);
    }

    size_t line_start(size_t r) const {
        assertIndex(r, lines() - 1);
        if (r == 0) {
            return 0;
        }
        size_t i = 0;
        for (size_t j = 0; j < r; ++j) {
            i = s.find('\n', i) + 1;
        }
        return i;
    }

    size_t line_length(size_t r) const {
        assertIndex(r, lines() - 1);
        if (r == lines() - 1) {
            return size() - line_start(r);
        }
        return line_start(r + 1) - line_start(r);
    }

    size_t char_to_line(size_t i) const {
        assertIndex(i);
        size_t r = 0;
        for (size_t j = 0; j < i; ++j) {
            if (s[j] == '\n') {
                ++r;
            }
        }
        return r;
    }

    std::string s;
};