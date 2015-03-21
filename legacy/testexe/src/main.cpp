#ifdef _WIN32
#include <Windows.h>
#include <Shlobj.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <sstream>
#include "medprotocol.h"
#include "corprotocol.h"

using namespace UDP;

int test1()
{
  const int frameDataLen = 6096;
  dg_byte_t *frameData = new dg_byte_t[frameDataLen];
  dg_byte_t val = 0;
  for (int i = 0; i < frameDataLen; ++i) {
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
    int completeSize = 0;
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
#if defined(_WIN32)
  FILE *file;
  fopen_s(&file, inputFilePath, "rb");
#elif defined(__linux__)
  FILE *file = fopen(inputFilePath, "rb");
#endif
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
  for (int i = 0; i < datagramsLength; ++i) {
    std::stringstream ss;
    ss << datagramsDir << "/datagram-" << i << ".bin";
    std::string filePath = ss.str();
#if defined(_WIN32)
    errno_t err = fopen_s(&file, filePath.c_str(), "wb");
    if (err != 0) {
      fclose(file);
      continue;
    }
#elif defined(__linux__)
    file = fopen(filePath.c_str(), "wb");
#endif
    datagrams[i]->write(file);
    fclose(file);
  }
  VideoFrameDatagram::freeData(datagrams, datagramsLength);
  delete[] fileData;

  // Read datagrams from disk.
  int writtenDatagrams = datagramsLength;
  dg_size_t readSize = 0;
  datagrams = new VideoFrameDatagram*[writtenDatagrams];
  for (int i = 0; i < writtenDatagrams; ++i) {
    std::stringstream ss;
    ss << datagramsDir << "/datagram-" << i << ".bin";
    std::string filePath = ss.str();
#if defined(_WIN32)
    fopen_s(&file, filePath.c_str(), "rb");
#elif defined(__linux__)
    file = fopen(filePath.c_str(), "rb");
#endif
    datagrams[i] = new VideoFrameDatagram();
    datagrams[i]->read(file);
    fclose(file);
    readSize += datagrams[i]->size;
  }

  // Write back to another file.
#if defined(_WIN32)
  fopen_s(&file, outputFile, "wb");
#elif defined(__linux__)
  fopen(outputFile, "wb");
#endif
  for (int i = 0; i < writtenDatagrams; ++i) {
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
  // Parse.
  cor_parser_settings sett;
  sett.on_frame_begin = [] (cor_parser *parser) -> int {
    return 0;
  };
  sett.on_frame_body_data = [] (cor_parser *parser, const uint8_t *data, size_t len) -> int {
    return 0;
  };
  sett.on_frame_end = [] (cor_parser *parser) -> int {
    return 0;
  };

  cor_parser *parser = (cor_parser*)malloc(sizeof(cor_parser));
  cor_parser_init(parser);

  while (true) {
    // Create request.
    cor_frame *req = (cor_frame*)malloc(sizeof(cor_frame));
    req->version = 1;
    req->type = cor_frame::TYPE_REQUEST;
    req->flags = 0;
    req->correlation_id = 234;
    req->length = 1024;
    req->data = (uint8_t*)malloc(req->length);
    for (int i = 0; i < req->length; ++i) {
      req->data[i] = 8;
    }

    // Serialize into a single buffer.
    size_t bufferlen = cor_frame::MINSIZE + req->length;
    uint8_t *buffer = (uint8_t*)malloc(bufferlen);
    uint8_t *p = buffer;
    memcpy(p, &req->version, sizeof(cor_frame::version_t));
    p += sizeof(cor_frame::version_t);
    memcpy(p, &req->type, sizeof(cor_frame::type_t));
    p += sizeof(cor_frame::type_t);
    memcpy(p, &req->flags, sizeof(cor_frame::flags_t));
    p += sizeof(cor_frame::flags_t);
    memcpy(p, &req->correlation_id, sizeof(cor_frame::correlation_t));
    p += sizeof(cor_frame::correlation_t);
    memcpy(p, &req->length, sizeof(cor_frame::data_length_t));
    p += sizeof(cor_frame::data_length_t);
    memcpy(p, req->data, req->length);
    p += req->length;

    cor_parser_parse(parser, sett, buffer, bufferlen);

    // Free resources.
    free(buffer);
    free(req->data);
    free(req);
  }

  free(parser);
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
