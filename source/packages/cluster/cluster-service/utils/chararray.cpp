#include "chararray.h"
#include "macrodefine.h"
#include <string.h>
#include <assert.h>

CharArray::CharArray()
{

}

CharArray::CharArray(int length)
{
    m_len = length;
    m_data = new char[length]();
}

CharArray::CharArray(const CharArray &charArray) //深拷贝，复制构造函数
{
    m_len = charArray.getLength();
    m_data = new char[m_len];
    memcpy(m_data, charArray.getData(), m_len);
}

CharArray::CharArray(CharArray &&charArray) noexcept //浅拷贝，移动构造函数
{
    m_data = charArray.getData();
    m_len = charArray.getLength();
    charArray.m_data = nullptr;
    charArray.m_len = 0;
}

CharArray& CharArray::operator=(const CharArray& charArray)
{
    if (this == &charArray) {
        return *this;
    }

    SAFE_DELETE_ARRAY(m_data);
    m_len = charArray.getLength();
    m_data = new char[m_len];
    memcpy(m_data, charArray.getData(), m_len);

    return *this;
}

CharArray& CharArray::operator=(CharArray&& charArray) noexcept
{
    if (this == &charArray) {
        return *this;
    }

    SAFE_DELETE_ARRAY(m_data);
    m_data = charArray.getData();
    m_len = charArray.getLength();
    charArray.m_data = nullptr;
    charArray.m_len = 0;

    return *this;
}

char CharArray::operator[](int i) const
{
    assert(m_data && i < m_len);

    return m_data[i];
}

bool CharArray::isNull() const
{
    return m_data == nullptr;
}

void CharArray::copyData(const void *src, int offset, int len)
{
    assert(m_data && (offset + len <= m_len));
    memcpy(m_data + offset, src, len);
}

CharArray::~CharArray()
{
    SAFE_DELETE_ARRAY(m_data);
    m_len = 0;
}

char* CharArray::getData() const
{
    return m_data;
}

int CharArray::getLength() const
{
    return m_len;
}
