#include <client_generated.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

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

  void encodePerson() {
    auto [person_name, person_age, person_weight, person_is_male] =
        data_src.get_next_person();

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

  void encodeGroup() {
    auto [grp_name, grp_avg_age, grp_avg_weight, grp_names_list] =
        data_src.get_next_group();

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
  std::tuple<std::string, float, float, bool> get_next_person() {
    return std::make_tuple("Ram", 21, 76.5, true);
  }
  std::tuple<std::string, float, float, std::vector<std::string>>
  get_next_group() {
    return std::make_tuple(
        std::string("FightClub"), 24.5, 66,
        std::vector<std::string>{"Ram", "Shayam", "Raghuveer"});
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

  void write(const char *data, std::size_t size) {
    if (file_stream.is_open()) {
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
  // Hard coded in assignment, Should not be done in production

  DummySource data_source;
  FileSink data_sink(argv[1]);

  ClientEncoder encoder(std::move(data_source), std::move(data_sink));

  encoder.encodePerson();
  encoder.encodeGroup();
  return EXIT_SUCCESS;
}
