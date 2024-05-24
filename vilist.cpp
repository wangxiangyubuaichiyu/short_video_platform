#include "vilist.h"

ViList::ViList()
{
    header=NULL;
    end=NULL;
    num=0;
}

ViList::~ViList()
{
    while(header)
    {
        VNode* t=header->next;
        delete header;
        header=t;
    }
}

void ViList::push_back(VNode *p)
{
    if(header==NULL)
    {
        end=header=p;
    }
    else
    {
        end->next=p;
        end=p;
    }
    num++;
}

void ViList::pop(int id)
{
    VNode* t=header;
    VNode* upt=NULL;
    while(t==NULL||t->Id!=id)
    {
        upt=t;
        t=t->next;
    }
    if(t==NULL)return;
    else
    {
        upt->next=t->next;
        if(t==end) end=upt;
        delete t;
    }
    num--;
}

void ViList::change(int id)
{

}

VNode *ViList::check(int id)
{
    VNode* t=header;
    while(t!=NULL&&t->Id!=id)
    {
        t=t->next;
    }
    if(t==NULL) return NULL;
    else return t;
}

int ViList::getNum() const
{
    return num;
}
