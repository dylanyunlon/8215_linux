#ifndef CHARARRAY_H
#define CHARARRAY_H

class CharArray
{
public:
    CharArray();
    ~CharArray();
    CharArray(int length);
    CharArray(const CharArray &charArray);
    CharArray(CharArray &&charArray) noexcept;
    CharArray &operator=(const CharArray& charArray);
    CharArray &operator=(CharArray&& charArray) noexcept;
    char operator[](int i) const;

    bool isNull() const;

    void copyData(const void *src, int offset, int len);
    char* getData() const;
    int getLength() const; //获取长度

private:
    char *m_data = nullptr;
    int m_len = 0;
};

#endif // CHARARRAY_H
