#include "money.h"
#include <cmath>

Money::Money(int64_t amountInKopecks)
    : m_amount(amountInKopecks) {}

int64_t Money::amount() const {
    return m_amount;
}

Money Money::operator+(const Money& other) const {
    return Money(m_amount + other.m_amount);
}

Money Money::operator-(const Money& other) const {
    return Money(m_amount - other.m_amount);
}

Money& Money::operator+=(const Money& other) {
    m_amount += other.m_amount;
    return *this;
}

Money& Money::operator-=(const Money& other) {
    m_amount -= other.m_amount;
    return *this;
}

bool Money::operator==(const Money& other) const {
    return m_amount == other.m_amount;
}

bool Money::operator!=(const Money& other) const {
    return m_amount != other.m_amount;
}

bool Money::operator<(const Money& other) const {
    return m_amount < other.m_amount;
}

bool Money::operator<=(const Money& other) const {
    return m_amount <= other.m_amount;
}

bool Money::operator>(const Money& other) const {
    return m_amount > other.m_amount;
}

bool Money::operator>=(const Money& other) const {
    return m_amount >= other.m_amount;
}

QString Money::toString() const {
    double displayAmount = static_cast<double>(m_amount) / 100.0;
    return QString::number(displayAmount, 'f', 2) + " ₴";
}

Money Money::fromString(const QString& str) {
    QString cleanStr = str;
    cleanStr.replace(',', '.');
    bool ok = false;
    double val = cleanStr.toDouble(&ok);
    if (!ok) return Money(0);
    return Money(static_cast<int64_t>(std::round(val * 100.0)));
}

Money operator"" _UAH(long double amount) {
    int64_t kopecks = static_cast<int64_t>(std::round(amount * 100.0));
    return Money(kopecks);
}

Money operator"" _UAH(unsigned long long amountInKopecks) {
    return Money(static_cast<int64_t>(amountInKopecks));
}

Money Money::operator*(int multiplier) const {
    return Money(m_amount * multiplier);
}

Money Money::operator*(double multiplier) const {
    return Money(static_cast<int64_t>(std::round(m_amount * multiplier)));
}

Money operator*(int multiplier, const Money& money) {
    return money * multiplier;
}

Money operator*(double multiplier, const Money& money) {
    return money * multiplier;
}
