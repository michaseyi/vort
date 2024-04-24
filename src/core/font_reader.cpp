#include <array>
#include <cassert>
#include <coroutine>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <vector>

template <typename T>
class Range {
 public:
  struct promise_type {
    T current_value;
    auto get_return_object() {
      return std::coroutine_handle<promise_type>::from_promise(*this);
    }

    auto initial_suspend() { return std::suspend_always{}; }
    auto final_suspend() noexcept { return std::suspend_always{}; }
    void return_void() {}
    void unhandled_exception() { std::terminate(); }
    auto yield_value(T value) {
      current_value = value;
      return std::suspend_always{};
    }
  };

  Range(std::coroutine_handle<promise_type> coroutine_handle)
      : coroutine_handle_(coroutine_handle) {}

  Range& begin() {
    if (!coroutine_handle_.done()) {
      coroutine_handle_.resume();
    }
    return *this;
  }

  std::default_sentinel_t end() { return {}; }

  T& operator*() { return coroutine_handle_.promise().current_value; }

  bool operator==(std::default_sentinel_t) const {
    return coroutine_handle_.done();
  }

  Range& operator++() {
    if (!coroutine_handle_.done()) {
      coroutine_handle_.resume();
    }
    return *this;
  }

 private:
  std::coroutine_handle<promise_type> coroutine_handle_;
};

void swap_endiness(uint16_t& value) {
  value = (value >> 8) | (value << 8);
}

void swap_endiness(int16_t& value) {
  value = (value >> 8) | (value << 8);
}

void swap_endiness(uint32_t& value) {
  value = (value >> 24) | ((value << 8) & 0x00FF0000) |
          ((value >> 8) & 0x0000FF00) | (value << 24);
}
void swap_endiness(int32_t& value) {
  value = (value >> 24) | ((value << 8) & 0x00FF0000) |
          ((value >> 8) & 0x0000FF00) | (value << 24);
}

struct FontHeader {
  uint32_t scaler_type;
  uint16_t num_tables;
  uint16_t search_range;
  uint16_t entry_selector;
  uint16_t range_shift;
};

struct TableDirectory {
  std::array<char, 4> tag;
  uint32_t checksum;
  uint32_t offset;
  uint32_t length;
};
struct GlyphHeader {
  int16_t number_of_contours;
  float x_min;
  float y_min;
  float x_max;
  float y_max;
};

struct GlyphData {
  std::vector<int16_t> contour_end_points;
  std::vector<uint8_t> flags;
  std::vector<int16_t> x_coordinates;
  std::vector<int16_t> y_coordinates;
};

enum struct GlyphFlag : uint32_t {
  OnCurve = 0,
  XShortVector = 1,
  YShortVector = 2,
  Repeat = 3,
  XIsSame = 4,
  YIsSame = 5,
  Reserved1 = 6,
  Reserved2 = 7,
};

enum struct MaximumProfileVersion : uint32_t {
  Version0 = 0x00005000,
  Version1 = 0x00010000,
};

enum struct Coordinate {
  X,
  Y,
};

enum struct LocationIndexFormat : int16_t {
  Short = 0,
  Long = 1,
};

class FontReader {
 public:
  FontReader(std::filesystem::path font_path) : font_path_(font_path) {
    font_file_.open(font_path_, std::ios::binary | std::ios::in);

    if (!font_file_.is_open()) {
      throw std::runtime_error("Failed to open font file");
    }
  }

  ~FontReader() {
    if (font_file_.is_open()) {
      font_file_.close();
    }
  }

  GlyphHeader read_glyph_header() {

    GlyphHeader header;
    header.number_of_contours = read_uint16();
    header.x_min = read_int16();
    header.y_min = read_int16();
    header.x_max = read_int16();
    header.y_max = read_int16();

    return header;
  }

  int32_t read_int32() {
    int32_t value;
    font_file_.read(reinterpret_cast<char*>(&value), sizeof(int32_t));
    swap_endiness(value);
    return value;
  }

  int16_t read_int16() {
    int16_t value;
    font_file_.read(reinterpret_cast<char*>(&value), sizeof(int16_t));
    swap_endiness(value);
    return value;
  }

  uint16_t read_uint16() {
    uint16_t value;
    font_file_.read(reinterpret_cast<char*>(&value), sizeof(uint16_t));
    swap_endiness(value);
    return value;
  }

  uint8_t read_uint8() {
    uint8_t value;
    font_file_.read(reinterpret_cast<char*>(&value), sizeof(uint8_t));
    return value;
  }

  uint32_t read_uint32() {
    uint32_t value;
    font_file_.read(reinterpret_cast<char*>(&value), sizeof(uint32_t));
    swap_endiness(value);
    return value;
  }

  TableDirectory find_table(const char* tag) {

    for (auto table : get_tables()) {
      if (std::equal(table.tag.begin(), table.tag.end(), tag)) {
        return table;
      }
    }

    throw std::runtime_error("TableDirectory not found");
  }

  uint32_t get_number_of_glyphs() {
    TableDirectory maxp_table = find_table("maxp");

    jump_to(maxp_table.offset);

    uint32_t num_glyphs = 0;
    switch (static_cast<MaximumProfileVersion>(read_uint32())) {
      case MaximumProfileVersion::Version0: {
        num_glyphs = read_uint16();
        break;
      }
      case MaximumProfileVersion::Version1: {
        num_glyphs = read_uint16();
        break;
      }

      default: {
        throw std::runtime_error("Invalid maximum profile version");
      }
    }

    return num_glyphs;
  }

  LocationIndexFormat get_location_index_format() {
    TableDirectory loca_table = find_table("head");

    jump_to(loca_table.offset + 50);

    LocationIndexFormat index_format = LocationIndexFormat::Short;

    switch (read_uint16()) {
      case 0: {
        index_format = LocationIndexFormat::Short;
        break;
      }
      case 1: {
        index_format = LocationIndexFormat::Long;
        break;
      }
      default: {
        throw std::runtime_error("Invalid location index format");
      }
    }

    return index_format;
  }

  GlyphData parse_simple_glyph(uint16_t number_of_contours) {
    GlyphData glyph_data;
    glyph_data.contour_end_points.resize(number_of_contours);

    for (auto& end_point : glyph_data.contour_end_points) {
      end_point = read_uint16();
    }

    // skip instruction length and instructions
    skip_byes(read_uint16());

    uint32_t num_coordinates = glyph_data.contour_end_points.back() + 1;

    glyph_data.flags.resize(num_coordinates);
    glyph_data.x_coordinates.resize(num_coordinates);
    glyph_data.y_coordinates.resize(num_coordinates);

    for (auto i = 0u; i < num_coordinates; i++) {
      uint8_t flag = read_uint8();
      glyph_data.flags.emplace_back(flag);
      if (flag_bit_set(flag, GlyphFlag::Repeat)) {
        uint8_t repeat_count = read_uint8();

        for (auto j = 0u; j < repeat_count; j++) {
          glyph_data.flags.emplace_back(flag);
          i++;
        }
      }
    }

    glyph_data.x_coordinates =
        read_coordinates(glyph_data.flags, Coordinate::X);
    glyph_data.y_coordinates =
        read_coordinates(glyph_data.flags, Coordinate::Y);
    return glyph_data;
  }

  std::vector<int16_t> read_coordinates(std::vector<uint8_t>& flags,
                                        Coordinate coordinate) {

    GlyphFlag coordinate_short_vector_flag = coordinate == Coordinate::X
                                                 ? GlyphFlag::XShortVector
                                                 : GlyphFlag::YShortVector;

    GlyphFlag coordinate_same_flag =
        coordinate == Coordinate::X ? GlyphFlag::XIsSame : GlyphFlag::YIsSame;

    std::vector<int16_t> coordinates(flags.size());

    for (auto i = 0u; i < flags.size(); i++) {
      coordinates.emplace_back(coordinates.empty() ? 0 : coordinates.back());

      uint8_t flag = flags[i];

      bool on_curve = flag_bit_set(flag, GlyphFlag::OnCurve);

      auto sign = flag_bit_set(flag, coordinate_short_vector_flag) &&
                          !flag_bit_set(flag, coordinate_same_flag)
                      ? -1
                      : 1;
    }

    return coordinates;
  }

  void parse_compound_glyph() {}

  void read_glyph() {
    uint32_t num_glyphs = get_number_of_glyphs();

    TableDirectory loca_table = find_table("loca");

    TableDirectory glyf_table = find_table("glyf");

    if (get_location_index_format() == LocationIndexFormat::Short) {
      throw std::runtime_error(
          "Short location index format yet to be implemented");
    }

    std::vector<uint32_t> offsets(num_glyphs + 1);

    jump_to(loca_table.offset);

    for (auto& offset : offsets) {
      offset = read_uint32();
    }

    for (auto i = 0u; i < offsets.size(); i++) {
      auto ii = i < offsets.size() - 1 ? i + 1 : num_glyphs;
      if (i == ii) {
        continue;
      }

      jump_to(glyf_table.offset + offsets[i]);

      GlyphHeader header = read_glyph_header();

      if (header.number_of_contours < 0) {
        parse_compound_glyph();
      } else if (header.number_of_contours > 0) {
        auto data = parse_simple_glyph(header.number_of_contours);
      }
    }
  }
  bool flag_bit_set(uint32_t flag, GlyphFlag bit_index) {
    return ((flag >> static_cast<uint32_t>(bit_index)) & 1) == 1;
  }
  void jump_to(uint32_t offset) { font_file_.seekg(offset, std::ios::beg); }

  void skip_byes(uint32_t bytes) { font_file_.seekg(bytes, std::ios::cur); }

  TableDirectory read_table() {
    assert(font_header_.has_value() && "Read header first");

    TableDirectory table;
    table.tag[0] = read_uint8();
    table.tag[1] = read_uint8();
    table.tag[2] = read_uint8();
    table.tag[3] = read_uint8();

    table.checksum = read_uint32();
    table.offset = read_uint32();
    table.length = read_uint32();

    return table;
  }

  Range<TableDirectory> get_tables() {
    FontHeader header = read_header();
    for (auto i = 0u; i < header.num_tables; i++) {
      auto table = read_table();
      co_yield table;
    }
  }

  FontHeader read_header() {
    jump_to(0);
    if (font_header_.has_value()) {
      jump_to(sizeof(FontHeader));
      return font_header_.value();
    }
    FontHeader font_header;

    font_header.scaler_type = read_uint32();
    font_header.num_tables = read_uint16();
    font_header.search_range = read_uint16();
    font_header.entry_selector = read_uint16();
    font_header.range_shift = read_uint16();

    font_header_ = font_header;
    return font_header;
  }

 private:
  std::optional<FontHeader> font_header_;
  std::filesystem::path font_path_;
  std::ifstream font_file_;
};

int test_font_reader(int argc, char** argv) {

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <font file>" << std::endl;
    return 1;
  }

  std::filesystem::path font_path = argv[1];
  FontReader font_reader(font_path);

  font_reader.read_glyph();

  return 0;
}
