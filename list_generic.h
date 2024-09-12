/* Generic module for dynamic array using standard library allocator
 * G_Container: G_Container type name
 * G_Type:      Contained type
 * G_Prefix:    G_Container function prefix
*/
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct G_Container G_Container;

#define Glue_0(A, B) A##_##B
#define Glue_1(A, B) Glue_0(A, B)
#define Glue_2(A, B) Glue_1(A, B)
#define Glue_(A, B)  Glue_2(A, B)

#define G_Container_Func(Name) \
    Glue_(G_Prefix, Name)

#define G_Container_Func_Decl(Ret, Name) \
	Ret Glue_(G_Prefix, Name)

struct G_Container {
	G_Type* data;
	int len;
	int cap;
};

G_Container_Func_Decl(bool, resize) (G_Container* ls, int new_cap){
	if(new_cap < 0){ return false; }
	
	int new_len = (ls->len < new_cap);
	G_Type* new_data = calloc(new_cap, sizeof(*new_data));
	if(new_data == NULL){ return false; }
	
	memcpy(new_data, ls->data, sizeof(*new_data) * new_len);
	free(ls->data);
	ls->data = new_data;
	ls->cap = new_cap;
	ls->len = new_len;
	return true;
}

G_Container_Func_Decl(bool, push) (G_Container* ls, G_Type element){
	if(ls->len >= ls->cap){
		if(!G_Container_Func(resize)(ls, (ls->cap + sizeof(element)) * 2)){
			return false;
		}
	}
	
	ls->data[ls->len] = element;
	ls->len += 1;
	return true;
}

G_Container_Func_Decl(bool, pop) (G_Container* ls){
	if(ls->len <= 0){
		return false;
	}
	ls->len -= 1;
	return true;
}

G_Container_Func_Decl(bool, shrink) (G_Container* ls){
	return G_Container_Func(resize)(ls, ls->len);
}

G_Container_Func_Decl(bool, pop_into) (G_Container* ls, G_Type* out){
	if(ls->len <= 0){
		return false;
	}
	ls->len -= 1;
	*out = ls->data[ls->len];
	return true;
}

G_Container_Func_Decl(G_Container, make) (int capacity){
	G_Container list = {0};
	if(capacity <= 0){ return list;	}
	
	list.data = calloc(capacity, sizeof(*list.data));
	list.cap = capacity;
	list.len = 0;
	return list;
}

G_Container_Func_Decl(void, destroy) (G_Container* list){
	G_Container clean = {0};
	free(list->data);
	*list = clean;
}

#undef Glue_0
#undef Glue_1
#undef Glue_2
#undef Glue_
#undef G_Container_Func_Decl
#undef G_Container
#undef G_Prefix
#undef G_Type