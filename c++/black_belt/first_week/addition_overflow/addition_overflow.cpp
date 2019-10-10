#include <iostream>
#include <optional>

using namespace std;

optional<int64_t> sum(int64_t lhs, int64_t rhs) {
    if (lhs > 0 && rhs > 0) {
        return lhs <= INT64_MAX - rhs ? make_optional<int64_t>(lhs + rhs) : nullopt;
    }
    if (lhs < 0 && rhs < 0) {
        return lhs >= INT64_MIN - rhs ? make_optional<int64_t>(lhs + rhs) : nullopt;
    }
    return make_optional<int64_t>(lhs + rhs);
}

int main() {
    int64_t lhs, rhs;
    cin >> lhs >> rhs;
    auto result = sum(lhs, rhs);
    if (result) {
        cout << result.value() << endl;
    } else {
        cout << "Overflow!" << endl;
    }
    return 0;
}
