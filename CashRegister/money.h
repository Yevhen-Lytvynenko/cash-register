#pragma once
#include <cstdint>
#include <QString>

class Money {
private:
    int64_t m_amount;

public:
    explicit Money(int64_t amountInKopecks = 0);

    [[nodiscard]] int64_t amount() const;

    Money operator+(const Money& other) const;
    Money operator-(const Money& other) const;
    Money operator*(int multiplier) const;
    Money operator*(double multiplier) const;
    Money& operator+=(const Money& other);
    Money& operator-=(const Money& other);

    bool operator==(const Money& other) const;
    bool operator!=(const Money& other) const;
    bool operator<(const Money& other) const;
    bool operator<=(const Money& other) const;
    bool operator>(const Money& other) const;
    bool operator>=(const Money& other) const;

    [[nodiscard]] QString toString() const;
    static Money fromString(const QString& str);
};

Money operator"" _UAH(long double amount);
Money operator"" _UAH(unsigned long long amountInKopecks);
Money operator*(int multiplier, const Money& money);
Money operator*(double multiplier, const Money& money);
