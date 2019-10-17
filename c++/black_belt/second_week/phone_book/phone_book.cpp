#include "phone_book.h"

PhoneBook::PhoneBook(std::vector<Contact> contacts) : contacts_(std::move(contacts)) {
        std::sort(contacts_.begin(), contacts_.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.name < rhs.name;
        });

        for (const auto& contact : contacts_) {
            PhoneBookSerialize::Contact proto_contact;
            proto_contact.set_name(contact.name);
            if (contact.birthday) {
                proto_contact.mutable_birthday()->set_day(contact.birthday->day);
                proto_contact.mutable_birthday()->set_month(contact.birthday->month);
                proto_contact.mutable_birthday()->set_year(contact.birthday->year);
            }
            for (const auto& phone_number : contact.phones) {
                *proto_contact.add_phone_number() = phone_number;
            }
            *proto_contacts.add_contact() = proto_contact;
        }
}

PhoneBook::PhoneBook(PhoneBookSerialize::ContactList &contact_list) {
    contacts_ = contactsFromProto(contact_list);
    proto_contacts = contact_list;
}

std::vector<Contact> PhoneBook::contactsFromProto(PhoneBookSerialize::ContactList& contact_list) {
    std::vector<Contact> contacts;
    for (const auto& contact : *contact_list.mutable_contact()) {
        Contact contact_;
        contact_.name = contact.name();
        if (contact.has_birthday()) {
            contact_.birthday = Date{};
            contact_.birthday->day = contact.birthday().day();
            contact_.birthday->month = contact.birthday().month();
            contact_.birthday->year = contact.birthday().year();
        }

        for (const auto& phone : contact.phone_number()) {
            contact_.phones.push_back(phone);
        }
        contacts.push_back(contact_);
    }
    return contacts;
}

PhoneBook::ContactRange PhoneBook::FindByNamePrefix(std::string_view name_prefix) const {
    auto begin = std::lower_bound(contacts_.begin(), contacts_.end(), name_prefix, cmp1);
    auto end = std::upper_bound(begin, contacts_.end(), name_prefix, cmp2);
    return ContactRange{begin, end};
}

void PhoneBook::SaveTo(std::ostream& output) const {
    proto_contacts.SerializeToOstream(&output);
}

PhoneBook DeserializePhoneBook(std::istream& input) {
    PhoneBookSerialize::ContactList contact_list;
    contact_list.ParseFromIstream(& input);
    return PhoneBook{contact_list};
}
