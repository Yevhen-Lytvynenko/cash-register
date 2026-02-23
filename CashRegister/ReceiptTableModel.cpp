#include "ReceiptTableModel.h"

ReceiptItem::ReceiptItem() : m_name(""), m_price(Money(0)), m_quantity(0) {}

ReceiptItem::ReceiptItem(QString name, Money price, int quantity)
    : m_name(std::move(name)), m_price(price), m_quantity(quantity > 0 ? quantity : 1) {}

QString ReceiptItem::name() const { return m_name; }
Money ReceiptItem::price() const { return m_price; }
int ReceiptItem::quantity() const { return m_quantity; }

void ReceiptItem::setQuantity(int quantity) {
    if (quantity > 0) {
        m_quantity = quantity;
    }
}

Money ReceiptItem::total() const { return m_price * m_quantity; }

ReceiptTableModel::ReceiptTableModel(QObject* parent)
    : QAbstractTableModel(parent) {}

void ReceiptTableModel::setItems(const std::vector<ReceiptItem>& items) {
    beginResetModel();
    m_items = items;
    endResetModel();
    emit totalsChanged();
}

int ReceiptTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_items.size());
}

int ReceiptTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return 4;
}

QVariant ReceiptTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_items.size())) {
        return {};
    }

    const auto& item = m_items[index.row()];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: return item.name();
        case 1: return item.quantity();
        case 2: return item.price().toString();
        case 3: return item.total().toString();
        default: return {};
        }
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() > 0) {
            return QVariant(static_cast<int>(Qt::AlignRight | Qt::AlignVCenter));
        }
        return QVariant(static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter));
    }

    return {};
}

QVariant ReceiptTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return QString("Назва товару");
        case 1: return QString("К-ть");
        case 2: return QString("Ціна");
        case 3: return QString("Сума");
        default: return {};
        }
    }
    return {};
}

void ReceiptTableModel::addItem(const ReceiptItem& item) {
    const int newRow = static_cast<int>(m_items.size());
    beginInsertRows(QModelIndex(), newRow, newRow);
    m_items.push_back(item);
    endInsertRows();
    emit totalsChanged();
}

void ReceiptTableModel::removeItem(int row) {
    if (row < 0 || row >= static_cast<int>(m_items.size())) return;

    beginRemoveRows(QModelIndex(), row, row);
    m_items.erase(m_items.begin() + row);
    endRemoveRows();
    emit totalsChanged();
}

void ReceiptTableModel::updateQuantity(int row, int newQuantity) {
    if (row < 0 || row >= static_cast<int>(m_items.size()) || newQuantity <= 0) return;

    m_items[row].setQuantity(newQuantity);
    const QModelIndex idx1 = index(row, 1);
    const QModelIndex idx2 = index(row, 3);
    emit dataChanged(idx1, idx2);
    emit totalsChanged();
}

ReceiptItem ReceiptTableModel::getItem(int row) const {
    if (row < 0 || row >= static_cast<int>(m_items.size())) return {};
    return m_items[row];
}

Money ReceiptTableModel::calculateSubtotal() const {
    Money subtotal(0);
    for (const auto& item : m_items) {
        subtotal += item.total();
    }
    return subtotal;
}

bool ReceiptTableModel::isEmpty() const {
    return m_items.empty();
}
