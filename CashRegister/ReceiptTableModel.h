#pragma once

#include <QAbstractTableModel>
#include <vector>
#include <QString>
#include "money.h"

class ReceiptItem {
public:
    ReceiptItem();
    ReceiptItem(QString name, Money price, int quantity);

    [[nodiscard]] QString name() const;
    [[nodiscard]] Money price() const;
    [[nodiscard]] int quantity() const;

    void setQuantity(int quantity);

    [[nodiscard]] Money total() const;

private:
    QString m_name;
    Money m_price;
    int m_quantity;
};

class ReceiptTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit ReceiptTableModel(QObject* parent = nullptr);

    void setItems(const std::vector<ReceiptItem>& items);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void addItem(const ReceiptItem& item);
    void removeItem(int row);
    void updateQuantity(int row, int newQuantity);

    [[nodiscard]] ReceiptItem getItem(int row) const;
    [[nodiscard]] Money calculateSubtotal() const;
    [[nodiscard]] bool isEmpty() const;

signals:
    void totalsChanged();

private:
    std::vector<ReceiptItem> m_items;
};
