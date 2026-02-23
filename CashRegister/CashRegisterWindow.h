#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_CashRegisterWindow.h"
#include "ReceiptTableModel.h"
#include "MacroManager.h"

class QButtonGroup;

class CashRegisterWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CashRegisterWindow(QWidget *parent = nullptr);
    ~CashRegisterWindow() override;

private slots:
    void on_btn_enter_clicked();
    void on_btn_clear_clicked();
    void on_btnDeleteItem_clicked();
    void on_btnApprove_clicked();
    void on_btnDecline_clicked();
    void onTotalsChanged();
    void onNumpadClicked(int id);

    void on_btnRecordMacro_clicked();
    void on_btnStopMacro_clicked();
    void on_btnPlayMacro_clicked();
    void on_btnPlayLoopMacro_clicked();

private:
    void setupNumpad();
    void updateFinancials();
    void resetPaymentState();
    void setupMacroUI();

    Ui::CashRegisterWindowClass ui;
    ReceiptTableModel* m_tableModel;
    Money m_tenderedAmount;
    MacroManager* m_macroManager;

    enum NumpadKeys {
        KeyBackspace = 10,
        KeyPoint = 11
    };
};
