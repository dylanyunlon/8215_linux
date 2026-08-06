#ifndef MACRODEFINE_H
#define MACRODEFINE_H
#define SAFE_DELETE(a)           {if (a){delete a; a = nullptr;}}
#define SAFE_DELETE_ARRAY(a)     {if (a){delete [] a; a = nullptr;}}
#endif // MACRODEFINE_H
