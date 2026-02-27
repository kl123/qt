#ifndef OVERVIEW_H
#define OVERVIEW_H

#include <QDialog>

namespace Ui {
class overview;
}

class overview : public QDialog
{
    Q_OBJECT

public:
    explicit overview(QWidget *parent = nullptr);
    ~overview();

private:
    Ui::overview *ui;
};

#endif // OVERVIEW_H
