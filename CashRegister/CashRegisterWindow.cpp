#include "CashRegisterWindow.h"
#include <QButtonGroup>
#include <QMessageBox>
#include <QRegularExpressionValidator>

CashRegisterWindow::CashRegisterWindow(QWidget *parent)
    : QMainWindow(parent),
    m_tableModel(new ReceiptTableModel(this)),
    m_tenderedAmount(0)
{
    ui.setupUi(this);

    std::vector<ReceiptItem> initialItems = {
        { "Еспресо", 35.00_UAH, 1 },
        { "Капучино велике", 55.00_UAH, 2 },
        { "Круасан з шоколадом", 65.50_UAH, 1 },
        { "Сирник", 70.00_UAH, 1 }
    };

    m_tableModel->setItems(initialItems);
    ui.receiptTableView->setModel(m_tableModel);

    ui.receiptTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui.receiptTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.receiptTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.receiptTableView->verticalHeader()->setVisible(false);
    ui.receiptTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.receiptTableView->setAlternatingRowColors(true);

    connect(m_tableModel, &ReceiptTableModel::totalsChanged, this, &CashRegisterWindow::onTotalsChanged);

    setupNumpad();

    QRegularExpression rx("^[0-9]{1,6}([.,][0-9]{1,2})?");
    QValidator *moneyValidator = new QRegularExpressionValidator(rx, this);
    ui.lineEdit->setValidator(moneyValidator);

    ui.verticalNumpadLayout->addStretch();
    ui.horizontalNumpadLayout->addStretch();

    ui.gridLayout->setContentsMargins(0, 20, 0, 20);

    updateFinancials();

    QString minimalistStyle = R"(
        QMainWindow {
            background-color: #F5F5F7;
        }
        QLabel {
            color: #1D1D1F;
            font-family: "Segoe UI", "Helvetica Neue", sans-serif;
        }
        QTableView {
            background-color: #FFFFFF;
            alternate-background-color: #FAFAFA;
            gridline-color: #EBEBEB;
            border: 1px solid #D2D2D7;
            border-radius: 8px;
            color: #1D1D1F;
            selection-background-color: #E8F0FE;
            selection-color: #1D1D1F;
        }
        QHeaderView::section {
            background-color: #FFFFFF;
            padding: 8px;
            border: none;
            border-bottom: 1px solid #D2D2D7;
            font-weight: 600;
            color: #86868B;
        }
        QPushButton {
            background-color: #FFFFFF;
            border: 1px solid #D2D2D7;
            border-radius: 8px;
            padding: 4px;
            color: #1D1D1F;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: #F5F5F7;
        }
        QPushButton:pressed {
            background-color: #EBEBEB;
        }
        QPushButton[text="0"], QPushButton[text="1"], QPushButton[text="2"],
        QPushButton[text="3"], QPushButton[text="4"], QPushButton[text="5"],
        QPushButton[text="6"], QPushButton[text="7"], QPushButton[text="8"],
        QPushButton[text="9"], QPushButton[text="."], QPushButton[text="<-"] {
            font-size: 22px;
            background-color: #FFFFFF;
            border: 1px solid #E5E5EA;
            padding: 0px;
        }
        QPushButton#btn_enter, QPushButton#btnApprove {
            background-color: #007AFF;
            color: white;
            border: none;
        }
        QPushButton#btn_enter:hover, QPushButton#btnApprove:hover { background-color: #0062CC; }
        QPushButton#btn_enter:pressed, QPushButton#btnApprove:pressed { background-color: #0051A8; }
        QPushButton#btn_clear, QPushButton#btnDecline, QPushButton#btnDeleteItem {
            background-color: transparent;
            color: #FF3B30;
            border: 1px solid #FF3B30;
        }
        QPushButton#btn_clear:hover, QPushButton#btnDecline:hover, QPushButton#btnDeleteItem:hover {
            background-color: #FFF0F0;
        }
        QPushButton#btn_clear:pressed, QPushButton#btnDecline:pressed, QPushButton#btnDeleteItem:pressed {
            background-color: #FFE5E5;
        }
        QLineEdit {
            background-color: #FFFFFF;
            border: 1px solid #D2D2D7;
            border-radius: 8px;
            padding: 8px;
            font-size: 20px;
            color: #1D1D1F;
        }
        QLineEdit:focus {
            border: 2px solid #007AFF;
        }
    )";

    this->setStyleSheet(minimalistStyle);
}

CashRegisterWindow::~CashRegisterWindow() = default;

void CashRegisterWindow::setupNumpad() {
    QButtonGroup* numpadGroup = new QButtonGroup(this);
    numpadGroup->addButton(ui.btnNumpad_0, 0);
    numpadGroup->addButton(ui.btnNumpad_1, 1);
    numpadGroup->addButton(ui.btnNumpad_2, 2);
    numpadGroup->addButton(ui.btnNumpad_3, 3);
    numpadGroup->addButton(ui.btnNumpad_4, 4);
    numpadGroup->addButton(ui.btnNumpad_5, 5);
    numpadGroup->addButton(ui.btnNumpad_6, 6);
    numpadGroup->addButton(ui.btnNumpad_7, 7);
    numpadGroup->addButton(ui.btnNumpad_8, 8);
    numpadGroup->addButton(ui.btnNumpad_9, 9);
    numpadGroup->addButton(ui.btnNumpadBackspace, KeyBackspace);
    numpadGroup->addButton(ui.btnNumpadPoint, KeyPoint);

    connect(numpadGroup, &QButtonGroup::idClicked, this, &CashRegisterWindow::onNumpadClicked);
}

void CashRegisterWindow::onNumpadClicked(int id) {
    QString currentText = ui.lineEdit->text();

    if (id == KeyBackspace) {
        if (!currentText.isEmpty()) {
            currentText.chop(1);
        }
    }
    else if (id == KeyPoint) {
        if (!currentText.contains(".") && !currentText.contains(",")) {
            currentText += (currentText.isEmpty() ? "0." : ".");
        }
    }
    else {
        if (currentText == "0") {
            currentText = QString::number(id);
        } else {
            currentText += QString::number(id);
        }
    }

    ui.lineEdit->setText(currentText);
}

void CashRegisterWindow::onTotalsChanged() {

    updateFinancials();
}

void CashRegisterWindow::updateFinancials() {
    Money subtotal = m_tableModel->calculateSubtotal();

    ui.labelSubtotal->setText(subtotal.toString());
    ui.labelAmountDue->setText(subtotal.toString());
    ui.labelTendered->setText(m_tenderedAmount.toString());

    if (m_tableModel->isEmpty() || subtotal.amount() == 0) {
        ui.labelChange->setText("0.00 ₴");
        ui.labelChange->setStyleSheet("");
        ui.btnApprove->setEnabled(false);
        return;
    }

    if (m_tenderedAmount < subtotal) {
        Money missing = subtotal - m_tenderedAmount;
        if (m_tenderedAmount.amount() > 0) {
            ui.labelChange->setText("Недостатньо коштів (-" + missing.toString() + ")");
            ui.labelChange->setStyleSheet("color: red; font-weight: bold;");
        } else {
            ui.labelChange->setText("0.00 ₴");
            ui.labelChange->setStyleSheet("");
        }
        ui.btnApprove->setEnabled(false);
    } else {
        Money change = m_tenderedAmount - subtotal;
        ui.labelChange->setText(change.toString());
        ui.labelChange->setStyleSheet("color: #007AFF; font-weight: bold;");
        ui.btnApprove->setEnabled(true);
    }
}

void CashRegisterWindow::resetPaymentState() {
    m_tenderedAmount = Money(0);
    ui.lineEdit->clear();
}

void CashRegisterWindow::on_btn_enter_clicked() {
    if (ui.lineEdit->text().isEmpty()) return;

    if (ui.receiptTableView->selectionModel()->hasSelection()) {
        int newQuantity = ui.lineEdit->text().toInt();
        if (newQuantity > 0) {
            int selectedRow = ui.receiptTableView->currentIndex().row();
            m_tableModel->updateQuantity(selectedRow, newQuantity);
        }
        ui.receiptTableView->clearSelection();
    } else {
        m_tenderedAmount = Money::fromString(ui.lineEdit->text());
        updateFinancials();
    }
    ui.lineEdit->clear();
}

void CashRegisterWindow::on_btn_clear_clicked() {
    ui.lineEdit->clear();
    ui.receiptTableView->clearSelection();
}

void CashRegisterWindow::on_btnDeleteItem_clicked() {
    if (ui.receiptTableView->selectionModel()->hasSelection()) {
        int selectedRow = ui.receiptTableView->currentIndex().row();
        m_tableModel->removeItem(selectedRow);
        ui.receiptTableView->clearSelection();
    }
}

void CashRegisterWindow::on_btnApprove_clicked() {
    Money subtotal = m_tableModel->calculateSubtotal();
    if (m_tenderedAmount < subtotal) return;

    if (QMessageBox::question(this, "Підтвердження", "Підтвердити оплату?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        m_tableModel->setItems({});
        resetPaymentState();
        updateFinancials();
    }
}

void CashRegisterWindow::on_btnDecline_clicked() {
    if (QMessageBox::question(this, "Відміна", "Скасувати поточний чек?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        m_tableModel->setItems({});
        resetPaymentState();
        updateFinancials();
    }
}
