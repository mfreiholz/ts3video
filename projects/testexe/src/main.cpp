#ifdef _WIN32
#include <Windows.h>
#include <Shlobj.h>
#endif
#include <string>
#include <cstdio>
#include <sstream>
#include "udpprotocol.h"
#include "tcpprotocol.h"

using namespace UDP;

int test1()
{
  auto frameDataLen = 6096;
  dg_byte_t *frameData = new dg_byte_t[frameDataLen];
  dg_byte_t val = 0;
  for (auto i = 0; i < frameDataLen; ++i) {
    if (i % VideoFrameDatagram::MAXSIZE == 0) {
      val += 1;
    } else if (i == frameDataLen - 1) {
      val = 255;
    }
    frameData[i] = val;
  }

  for (int z = 0; z < 500000; ++z) {
    // Encode.
    VideoFrameDatagram **datagrams = 0;
    VideoFrameDatagram::dg_data_count_t datagramsLength;
    VideoFrameDatagram::split(frameData, frameDataLen, 0, 0, &datagrams, datagramsLength);

    // Decode.
    auto completeSize = 0;
    for (int i = 0; i < datagramsLength; ++i) {
      completeSize += datagrams[i]->size;
    }

    // Clean up.
    VideoFrameDatagram::freeData(datagrams, datagramsLength);
  }

  delete[] frameData;
  return 0;
}

/*
  - Reads file from disk
  - Split it into datagrams.
  - Write datagrams to disk.
  - Read datagrams from disk.
  - Write merged datagram content into new file.
  - Compare files (manually with BeyondCompare).
*/
void test2()
{
  char inputFilePath[] = "C:/Temp/in";
#ifdef UDP_USE_HOSTBYTEORDER
  char datagramsDir[] = "C:/Temp/datagrams-hostbyteorder";
  char outputFile[] = "C:/Temp/out-h";
#else
  char datagramsDir[] = "C:/Temp/datagrams-networkbyteorder";
  char outputFile[] = "C:/Temp/out-n";
#endif

#ifdef WIN32
  std::stringstream rmdirCommand;
  rmdirCommand << "rmdir /S /Q \"" << datagramsDir << "\"";
  system(rmdirCommand.str().c_str());

  std::stringstream mkdirCommand;
  mkdirCommand << "mkdir \"" << datagramsDir << "\"";
  system(mkdirCommand.str().c_str());
#endif

  // Read file from disk into buffer.
  FILE *file;
  fopen_s(&file, inputFilePath, "rb");
  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);
  rewind(file);
  dg_byte_t *fileData = new dg_byte_t[fileSize];
  fread(fileData, 1, fileSize, file);
  fclose(file);

  // Split file content into datagrams.
  VideoFrameDatagram **datagrams = 0;
  VideoFrameDatagram::dg_data_count_t datagramsLength;
  if (VideoFrameDatagram::split(fileData, fileSize, 0, 0, &datagrams, datagramsLength) != 0) {
    return; // Error.
  }

  // Write datagrams to disk.
  for (auto i = 0; i < datagramsLength; ++i) {
    std::stringstream ss;
    ss << datagramsDir << "/datagram-" << i << ".bin";
    std::string filePath = ss.str();
    errno_t err = fopen_s(&file, filePath.c_str(), "wb");
    if (err == 0)
      datagrams[i]->write(file);
    fclose(file);
  }
  VideoFrameDatagram::freeData(datagrams, datagramsLength);
  delete[] fileData;

  // Read datagrams from disk.
  auto writtenDatagrams = datagramsLength;
  dg_size_t readSize = 0;
  datagrams = new VideoFrameDatagram*[writtenDatagrams];
  for (auto i = 0; i < writtenDatagrams; ++i) {
    std::stringstream ss;
    ss << datagramsDir << "/datagram-" << i << ".bin";
    std::string filePath = ss.str();
    fopen_s(&file, filePath.c_str(), "rb");
    datagrams[i] = new VideoFrameDatagram();
    datagrams[i]->read(file);
    fclose(file);
    readSize += datagrams[i]->size;
  }

  // Write back to another file.
  fopen_s(&file, outputFile, "wb");
  for (auto i = 0; i < writtenDatagrams; ++i) {
    fwrite(datagrams[i]->data, sizeof(dg_byte_t), datagrams[i]->size, file);
  }
  fclose(file);
  VideoFrameDatagram::freeData(datagrams, datagramsLength);
}

/*
  - Creates COR request
  - Serializes COR request
  - Parse COR request.
 */
void test3()
{
  while (true) {
    // Create request.
    cor_request_t *req = cor_create_request();
    req->version = 1;
    req->type = cor_request_t::TYPE_REQUEST;
    req->flags = 0;
    req->correlation_id = 234;
    req->length = 1024;
    req->data = (uint8_t*)malloc(req->length);
    for (int i = 0; i < req->length; ++i) {
      req->data[i] = 8;
    }

    // Serialize into a single buffer.
    size_t bufferlen = cor_request_t::MINSIZE + req->length;
    uint8_t *buffer = (uint8_t*)malloc(bufferlen);
    uint8_t *p = buffer;
    memcpy(p, &req->version, sizeof(cor_request_t::version_t));
    p += sizeof(cor_request_t::version_t);
    memcpy(p, &req->type, sizeof(cor_request_t::type_t));
    p += sizeof(cor_request_t::type_t);
    memcpy(p, &req->flags, sizeof(cor_request_t::flags_t));
    p += sizeof(cor_request_t::flags_t);
    memcpy(p, &req->correlation_id, sizeof(cor_request_t::correlation_t));
    p += sizeof(cor_request_t::correlation_t);
    memcpy(p, &req->length, sizeof(cor_request_t::data_length_t));
    p += sizeof(cor_request_t::data_length_t);
    memcpy(p, req->data, req->length);
    p += req->length;

    // Parse.
    cor_parser_settings_t sett;
    sett.on_request_begin = [] (cor_parser_t *parser, const uint8_t *data, size_t len) -> int {
      return 0;
    };
    sett.on_request_body_data = [] (cor_parser_t *parser, const uint8_t *data, size_t len) -> int {
      return 0;
    };
    sett.on_request_end = [] (cor_parser_t *parser, const uint8_t *data, size_t len) -> int {
      return 0;
    };

    cor_parser_t *parser = (cor_parser_t*)malloc(sizeof(cor_parser_t));
    cor_parser_init(parser);
    cor_parser_parse(parser, sett, buffer, bufferlen);
    free(parser);

    // Free resources.
    free(buffer);
    free(req->data);
    cor_free_request(req);
  }
}

int main(int argc, char **argv)
{
  test3();
  return 0;
}

/*int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
  while (true) {
    test2();
  }
  return 0;
}*/