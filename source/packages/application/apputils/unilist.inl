#include <algorithm>
#include "applog.h"

template<class Item>
Unilist<Item>::Unilist()
{
}

template<class Item>
Unilist<Item>::~Unilist()
{
}

template<class Item>
bool Unilist<Item>::has(const Item &ID) const
{
    bool ret = false;
    typename list<Item>::const_iterator iter;

    iter = find(list<Item>::begin(), list<Item>::end(), ID);
    if (list<Item>::end() != iter)
        ret = true;

    return ret;
}

template<class Item>
void Unilist<Item>::push_back(const Item &ID, unsigned int index)
{
    list<Item>::remove(ID);
    typename list<Item>::iterator it = list<Item>::end();

    if (index > list<Item>::size())
        LOGW("Unilist", "index: %d > size: %d, sure?\n", index, list<Item>::size());

    while (index-- > 0) {
        it--;
    }

    list<Item>::insert(it, ID);
}

