#ifndef UI_POWER_MANAGEMENT_DIALOG_H_
#define UI_POWER_MANAGEMENT_DIALOG_H_

#include "onyx/base/base.h"
#include "onyx/sys/sys.h"
#include "onyx_dialog.h"
#include "onyx/ui/catalog_view.h"

namespace ui
{

/// Locale widget. Not sure we should place it in the ui library.
/// So far, only explorer use it.
class PowerManagementDialog : public OnyxDialog
{
    Q_OBJECT

public:
    PowerManagementDialog(QWidget *parent, SysStatus & ref);
    ~PowerManagementDialog(void);

public:
    int exec();

private:
    void createLayout();
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    bool event(QEvent*);

    void setSuspendInterval();
    void setShutdownInterval();

private Q_SLOTS:
    void onButtonChanged(CatalogView *catalog, ContentView *item, int user_data);
    void onOkClicked();
    int profileIndex();

private:
    SysStatus & status_;
    QVBoxLayout ver_layout_;

    OnyxLabel battery_power_;

    CatalogView buttons_;
    OData *interval_selected_;


    QHBoxLayout hor_layout_;
    CatalogView ok_;

    int sys_standby_interval_;
    int standby_interval_;
    int sys_shutdown_interval_;     ///< From system manager.
    int shutdown_interval_;         ///< User specified.
};

}   // namespace ui

#endif      // UI_POWER_MANAGEMENT_DIALOG_H_
