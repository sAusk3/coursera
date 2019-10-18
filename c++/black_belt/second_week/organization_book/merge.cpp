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

    void SetRepeatedField(Company& company, const std::string& field_name, const Signals& signals, const Providers& providers) {
        Signals has_field = HasRepeatedField(company, field_name, signals, providers);
        if (!has_field.empty()) {
            for (auto& signal : has_field) {
                auto signal_company = signal.company();
                auto descriptor = getDescriptor(signal_company, field_name);
                auto reflection = signal_company.GetReflection();
                for (size_t i = 0; i < reflection->FieldSize(signal_company, descriptor); ++i) {
                    const auto& added_message = reflection->GetRepeatedMessage(signal_company, descriptor, i);
                    company.GetReflection()->AddMessage(&company, getDescriptor(company, field_name))->CopyFrom(added_message);
                }
            }
        }
    }

  Company Merge(const Signals& signals, const Providers& providers) {
      Company company;

      SetUniqueField(company, "working_time", signals, providers);
      SetUniqueField(company, "address", signals, providers);

      SetRepeatedField(company, "names", signals, providers);
      std::sort(company.mutable_names()->begin(), company.mutable_names()->end(), [](const YellowPages::Name& lhs,
                                                                                     const YellowPages::Name& rhs) {
          if (lhs.value() < rhs.value()) {
              return true;
          }
          if (rhs.value() > rhs.value()) {
              return false;
          }
          return lhs.type() < rhs.type();
      });
      company.mutable_names()->erase(std::unique(company.mutable_names()->begin(), company.mutable_names()->end(), [](const YellowPages::Name& lhs,
                                                                                                                      const YellowPages::Name& rhs) {
          return lhs.value() == rhs.value() && lhs.type() == rhs.type();
      }), company.mutable_names()->end());

      SetRepeatedField(company, "phones", signals, providers);
      std::sort(company.mutable_phones()->begin(), company.mutable_phones()->end(), [](const YellowPages::Phone& lhs,
              const YellowPages::Phone& rhs) {
          if (lhs.number() < rhs.number()) {
              return true;
          }
          if (lhs.number() > rhs.number()) {
              return false;
          }

          if (lhs.country_code() < rhs.country_code()) {
              return true;
          }
          if (lhs.country_code() > rhs.country_code()) {
              return false;
          }

          if (lhs.description() < rhs.description()) {
              return true;
          }
          if (lhs.description() > rhs.description()) {
              return false;
          }

          if (lhs.extension() < rhs.extension()) {
              return true;
          }
          if (lhs.extension() > rhs.extension()) {
              return false;
          }

          if (lhs.formatted() < rhs.formatted()) {
              return true;
          }
          if (lhs.formatted() > rhs.formatted()) {
              return false;
          }

          if (lhs.local_code() < rhs.local_code()) {
              return true;
          }
          if (lhs.local_code() > rhs.local_code()) {
              return false;
          }

          return lhs.type() < rhs.type();
      });
      company.mutable_phones()->erase(std::unique(company.mutable_phones()->begin(), company.mutable_phones()->end(), [](const YellowPages::Phone& lhs,
                                                                                                                      const YellowPages::Phone& rhs) {
          return lhs.number() == rhs.number() && lhs.country_code() == rhs.country_code() && lhs.description() == rhs.description() &&
                 lhs.extension() == rhs.extension() && lhs.formatted() == rhs.formatted() && lhs.local_code() == rhs.local_code() && lhs.type() == rhs.type();
      }), company.mutable_phones()->end());

      SetRepeatedField(company, "urls", signals, providers);
      std::sort(company.mutable_urls()->begin(), company.mutable_urls()->end(), [](const YellowPages::Url& lhs,
              const YellowPages::Url& rhs) {
          return lhs.value() < rhs.value();
      });
      company.mutable_urls()->erase(std::unique(company.mutable_urls()->begin(), company.mutable_urls()->end(), [](const YellowPages::Url& lhs,
      const YellowPages::Url& rhs) {
          return lhs.value() == rhs.value();
      }),  company.mutable_urls()->end());

      return company;
  }

}
