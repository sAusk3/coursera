#pragma once

#include "iterator_range.h"

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <iosfwd>
#include <contact.pb.h>

struct Date {
    int year, month, day;
};

struct Contact {
    std::string name;
    std::optional<Date> birthday;
    std::vector<std::string> phones;
};

class PhoneBook {
public:
    explicit PhoneBook(std::vector<Contact> contacts);

    explicit PhoneBook(PhoneBookSerialize::ContactList& contact_list);

    using ContactRange = IteratorRange<std::vector<Contact>::const_iterator>;
    ContactRange FindByNamePrefix(std::string_view name_prefix) const;

    void SaveTo(std::ostream& output) const;

    static std::vector<Contact> contactsFromProto(PhoneBookSerialize::ContactList& contact_list);

private:
    static bool cmp1(const Contact& contact, const std::string_view& prefix) {
        return contact.name < prefix && contact.name.find(prefix) != 0;
    }

    static bool cmp2(const std::string_view& prefix, const Contact& contact) {
        return contact.name.find(prefix);
    }

    std::vector<Contact> contacts_;
    PhoneBookSerialize::ContactList proto_contacts;
};

PhoneBook DeserializePhoneBook(std::istream& input);
