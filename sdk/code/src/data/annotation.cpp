#include "onyx/data/annotation.h"

namespace anno
{

Annotation::Annotation()
{
}

Annotation::Annotation(const Annotation & right)
    : title_(right.title_)
    , data_(right.data_)
    , rect_list_(right.rect_list_)
    , page_(right.page_)
{
}

Annotation::Annotation(const QString & title, const QVariant & data)
    : title_(title)
    , data_(data)
{
}

Annotation::~Annotation(void)
{
}

QDataStream & operator << ( QDataStream & out, const Annotation & annotation )
{
    out << annotation.data();
    out << annotation.title();
    out << annotation.rect_list();
    out << annotation.page();
    return out;
}

QDataStream & operator >> ( QDataStream & in, Annotation & annotation )
{
    in >> annotation.mutable_data();
    in >> annotation.mutable_title();
    in >> annotation.mutable_rect_list();
    in >> annotation.mutable_page();
    return in;
}

}
