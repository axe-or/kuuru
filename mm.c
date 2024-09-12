#include <stdio.h>
#include <stdbool.h>

#define G_Container List_Float
#define G_Type      float
#define G_Prefix    listf
#include "list_generic.h"


static void print_list(List_Float l){
	int i = 0;
	printf("len:%d cap:%d [ ", l.len, l.cap);
	for(i = 0; i < l.len; i ++){
		printf("%.2f ", l.data[i]);
	}
	printf("]\n");
}

int main(){
	List_Float l = listf_make(4);
	print_list(l);
    listf_push(&l, 6.9);
	print_list(l);
    listf_push(&l, 4.2);
    listf_push(&l, 0);
    print_list(l);
    listf_push(&l, 1.5);
    print_list(l);
    listf_push(&l, 42);
    print_list(l);
    listf_shrink(&l);
    print_list(l);
    listf_pop(&l);
    print_list(l);
    listf_resize(&l, 2);
    print_list(l);
    listf_pop(&l);
    print_list(l);
    listf_pop(&l);
    listf_pop(&l);
    print_list(l);
	return 0;
}