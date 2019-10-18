#include "yellow_pages.h"

#include <algorithm>
#include <tuple>

namespace YellowPages {
    static const google::protobuf::FieldDescriptor* getDescriptor(const Company& company, const std::string field_name) {
        return company.GetDescriptor()->FindFieldByName(field_name);
    }

    static Signals GetBestSignals(Signals& signals, const Providers& providers) {
        size_t max_priority = 0;
        for (const auto& signal : signals) {
            if (providers.at(signal.provider_id()).priority() > max_priority) {
                max_priority = providers.at(signal.provider_id()).priority();
            }
        }
        Signals signals_with_max_priority;
        for (const auto& signal : signals) {
            if (providers.at(signal.provider_id()).priority() == max_priority) {
                signals_with_max_priority.push_back(signal);
            }
        }
        return signals_with_max_priority;
    }

    static Signals HasSingleField(Company& company, const std::string& field_name, const Signals& signals, const Providers& providers) {
        Signals has_field;
        for (const auto& signal : signals) {
            const auto& signal_company = signal.company();
            if (signal_company.GetReflection()->HasField(signal_company, getDescriptor(signal_company, field_name))) {
                has_field.push_back(signal);
            }
        }
        return GetBestSignals(has_field, providers);
    }

    static Signals HasRepeatedField(Company& company, const std::string& field_name, const Signals& signals, const Providers& providers) {
        Signals has_field;
        for (const auto& signal : signals) {
            const auto& signal_company = signal.company();
            auto descriptor = getDescriptor(company, field_name);
            if (signal_company.GetReflection()->FieldSize(signal_company, descriptor) != 0) {
                has_field.push_back(signal);
            }
        }
        return GetBestSignals(has_field, providers);
    }

    static void SetUniqueField(Company& company, const std::string& field_name, const Signals& signals, const Providers& providers) {
        Signals has_field = HasSingleField(company, field_name, signals, providers);
        if (!has_field.empty()) {
            auto signal_company = has_field.front().mutable_company();
            auto message = company.GetReflection()->MutableMessage(&company, company.GetDescriptor()->FindFieldByName(field_name));
            message->CopyFrom(
                    signal_company->GetReflection()->GetMessage(*signal_company, signal_company->GetDescriptor()->FindFieldByName(field_name))
            );
        }
    }

    static void SetRepeatedField(Company& company, const std::string& field_name, const Signals& signals, const Providers& providers) {
        Signals has_field = HasRepeatedField(company, field_name, signals, providers);
        std::set<std::string> serialized_signals;
        if (!has_field.empty()) {
            for (auto& signal : has_field) {
                auto signal_company = signal.company();
                auto descriptor = getDescriptor(signal_company, field_name);
                auto reflection = signal_company.GetReflection();
                for (size_t i = 0; i < reflection->FieldSize(signal_company, descriptor); ++i) {
                    const auto& added_message = reflection->GetRepeatedMessage(signal_company, descriptor, i);
                    serialized_signals.insert(added_message.SerializeAsString());
                }
            }
            for (const auto& serialized_signal : serialized_signals) {
                auto msg = company.GetReflection()->AddMessage(&company, getDescriptor(company, field_name));
                msg->ParseFromString(serialized_signal);
            }
        }
    }

  Company Merge(const Signals& signals, const Providers& providers) {
      Company company;

      SetUniqueField(company, "working_time", signals, providers);
      SetUniqueField(company, "address", signals, providers);

      SetRepeatedField(company, "names", signals, providers);
      SetRepeatedField(company, "phones", signals, providers);
      SetRepeatedField(company, "urls", signals, providers);

      return company;
  }

}
