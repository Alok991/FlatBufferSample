#include <client_generated.h>
#include <fstream>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

namespace {
struct Person {
  std::string name;
  float age;
  float weight;
  bool is_male;
};

struct Group {
  std::string name;
  float avg_age;
  float avg_weight;
  std::vector<std::string> all_names;
};

typedef std::variant<std::monostate, Person, Group> ClientU;

} // namespace

// input data for encoder could be coming from any source, DataSource
// DataSource could be a json file, char[] or network stream
// DataSink is where we want to put the serialised data, this could be a binary
// file, network socket, SharedMemory etc
template <typename DataSource, typename DataSink> class ClientEncoder {
  DataSource data_src;
  DataSink data_sink;

public:
  ClientEncoder(DataSource &&data_src, DataSink &&data_sink)
      : data_src(std::forward<DataSource>(data_src)),
        data_sink(std::forward<DataSink>(data_sink)) {}

  void start_encoding() {
    while (true) { // this simulate inf data stream
      ClientU data = data_src.get_next_data();
      if (std::holds_alternative<Person>(data)) {
        auto person = std::get<Person>(data);
        encodePerson(person);
      } else if (std::holds_alternative<Group>(data)) {
        auto grp = std::get<Group>(data);
        encodeGroup(grp);
      } else {
        break;
      }
    }
  }

  void encodePerson(const Person &person_arg) {
    auto &&person_name = person_arg.name;
    auto &&person_age = person_arg.age;
    auto &&person_weight = person_arg.weight;
    auto &&person_is_male = person_arg.is_male;

    flatbuffers::FlatBufferBuilder builder;
    auto name = builder.CreateString(person_name);
    auto person = fb::CreatePerson(builder, name, person_age, person_weight,
                                   person_is_male ? fb::Gender::Gender_Male
                                                  : fb::Gender::Gender_Female);
    auto client = fb::CreateClient(builder, fb::ClientType::ClientType_Person,
                                   person.Union());
    builder.Finish(client);

    data_sink.write((const char *)builder.GetBufferPointer(),
                    builder.GetSize());
  }

  void encodeGroup(const Group &grp) {
    auto &&grp_name = grp.name;
    auto &&grp_avg_age = grp.avg_age;
    auto &&grp_avg_weight = grp.avg_weight;
    auto &&grp_names_list = grp.all_names;

    flatbuffers::FlatBufferBuilder builder;

    auto grp_name_serialised = builder.CreateString(grp_name);

    std::vector<flatbuffers::Offset<flatbuffers::String>> names_vector;
    for (auto &&name : grp_names_list) {
      auto serialised_name = builder.CreateString(name);
      names_vector.push_back(serialised_name);
    }
    auto names_list_serialised = builder.CreateVector(names_vector);

    auto group = fb::CreateGroup(builder, grp_name_serialised, grp_avg_age,
                                 grp_avg_weight, names_list_serialised);

    auto client = fb::CreateClient(builder, fb::ClientType::ClientType_Group,
                                   group.Union());
    builder.Finish(client);

    data_sink.write((const char *)builder.GetBufferPointer(),
                    builder.GetSize());
  }
};

struct DummySource {
private:
  int counter = 0;

public:
  ClientU get_next_data() {
    ClientU data = std::monostate{};

    if (counter == 0) {
      Person person;
      person.name = "Ram";
      person.age = 21;
      person.weight = 76.5;
      person.is_male = true;

      data = person;
    } else if (counter == 1) {
      Group grp;
      grp.name = "FlightClub";
      grp.avg_age = 24.5;
      grp.avg_weight = 66;
      grp.all_names = std::vector<std::string>{"Ram", "Shayam", "Raghuveer"};
      data = grp;
    }
    ++counter;
    return data;
  }
};

class FileSink {

public:
  FileSink(const std::string &filename) {
    file_stream.open(filename, std::ios::out | std::ios::binary);
    if (!file_stream.is_open()) {
      std::cerr << "Error: Could not open file for writing: " << filename
                << std::endl;
    }
  }

  ~FileSink() {
    if (file_stream.is_open()) {
      file_stream.flush(); // TODO check if close does flush
      file_stream.close();
    }
  }

  // copy not declared

  // Move constructor
  FileSink(FileSink &&other) noexcept { file_stream.swap(other.file_stream); }

  // Move assignment operator
  FileSink &operator=(FileSink &&other) noexcept {
    if (this != &other) {
      file_stream.swap(other.file_stream);
    }
    return *this;
  }

  void write(const char *data, uint64_t size) {
    if (file_stream.is_open()) {
      file_stream.write((const char *)&size, sizeof(uint64_t));
      file_stream.write(data, size);
    }
  }

  bool isOpen() const { return file_stream.is_open(); }

private:
  std::ofstream file_stream;
};

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage : ./path-to-fb_encoder "
                 "<path-to-fb_bytes.bin>\n";
    return EXIT_FAILURE;
  }

  DummySource data_source;
  FileSink data_sink(argv[1]);

  ClientEncoder encoder(std::move(data_source), std::move(data_sink));

  encoder.start_encoding();
  return EXIT_SUCCESS;
}
