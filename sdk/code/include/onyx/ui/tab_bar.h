#ifndef UI_LIB_TAB_BAR_H_
#define UI_LIB_TAB_BAR_H_

#include <vector>
#include "catalog_view.h"
#include "tab_button.h"

namespace ui
{

class TabBar : public CatalogView
{
    Q_OBJECT
public:
    TabBar(QWidget *parent);
    ~TabBar(void);

public:
    bool addButton(const int id, const QString & title, const QPixmap & pixmap);
    bool removeButton(const int id);
    bool clickButton(const int id);
    int  selectedButton();
    bool setButtonText(const int id, const QString & title);

    bool setOrientation(const Qt::Orientation orientation);
    Qt::Orientation orientation() const { return orientation_; }

Q_SIGNALS:
    void buttonClicked(int id);

private Q_SLOTS:
    virtual void onItemActivated(ContentView *item, int);
    void selectButton(const int id);
    void activateButton(const int id);

private:
    void clickSelectedChild();

private:
    Qt::Orientation orientation_;
};

};

#endif
