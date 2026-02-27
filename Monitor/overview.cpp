#include "overview.h"
#include "ui_overview.h"

overview::overview(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::overview)
{
    ui->setupUi(this);
}

overview::~overview()
{
    delete ui;
}
