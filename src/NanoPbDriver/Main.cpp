#include "NanoPb/Configuration.pb.h"
#include "DebugTrace.h"

#include <pb_encode.h>
#include <pb_decode.h>

#include <ntifs.h>

struct MemoryView
{
	void* data;
	size_t size;

	bool empty() const
	{
		return nullptr == data && size == 0;
	}

	static MemoryView getEmptyView()
	{
		return MemoryView{ nullptr, 0 };
	}
};

MemoryView ReadConfigurationFile()
{
	IO_STATUS_BLOCK ioStatusBlock;
	HANDLE fileHandle;
	LARGE_INTEGER byteOffset = { 0 };

	UNICODE_STRING configFilePath = RTL_CONSTANT_STRING(L"\\??\\C:\\temp\\config.bin");
	OBJECT_ATTRIBUTES objectAttributes =
		RTL_CONSTANT_OBJECT_ATTRIBUTES(&configFilePath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE);

	// Open the file
	NTSTATUS status = ZwCreateFile(&fileHandle,
		GENERIC_READ,
		&objectAttributes,
		&ioStatusBlock,
		nullptr,
		FILE_ATTRIBUTE_NORMAL,
		0,
		FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT,
		nullptr,
		0);

	if (!NT_SUCCESS(status)) {
		DbgPrint("Failed to open file: %08x\n", status);
		return MemoryView::getEmptyView();
	}

	// Query file size
	FILE_STANDARD_INFORMATION fileInfo;
	status = ZwQueryInformationFile(fileHandle,
		&ioStatusBlock,
		&fileInfo,
		sizeof(fileInfo),
		FileStandardInformation);

	if (!NT_SUCCESS(status)) {
		DbgPrint("Failed to query file information: %08x\n", status);
		ZwClose(fileHandle);
		return MemoryView{ nullptr, 0 };
	}

	DbgPrint("File size: %llu bytes\n", fileInfo.EndOfFile.QuadPart);

	// Allocate memory for the file content
	const size_t bufferSize = fileInfo.EndOfFile.QuadPart;
	void* buffer = ExAllocatePool(PagedPool, (SIZE_T)bufferSize);
	if (nullptr == buffer) {
		DbgPrint("Failed to allocate memory for file content\n");
		ZwClose(fileHandle);
		return MemoryView{ nullptr, 0 };
	}

	// Read the file
	status = ZwReadFile(fileHandle,
		nullptr,
		nullptr,
		nullptr,
		&ioStatusBlock,
		buffer,
		static_cast<ULONG>(bufferSize),
		&byteOffset,
		nullptr);

	if (!NT_SUCCESS(status))
	{
		DbgPrint("Failed to read file: %08x\n", status);
		ExFreePool(buffer);
		ZwClose(fileHandle);
		return MemoryView{ nullptr, 0 };
	}

	const ULONG bytesRead = (ULONG)ioStatusBlock.Information;
	DbgPrint("Read %lu bytes from file.\n", bytesRead);

	// Close the file handle
	ZwClose(fileHandle);

	return MemoryView{ buffer, bufferSize };
}

extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT, PUNICODE_STRING)
{
	const MemoryView config = ReadConfigurationFile();
	if (nullptr == config.data)
	{
		DEBUG_LOG("Failed to read config data");
		return STATUS_FAILED_DRIVER_ENTRY;
	}

	pb_istream_t stream = pb_istream_from_buffer((pb_byte_t*)config.data, config.size);

	SimpleMessage message = SimpleMessage_init_zero;
	const bool status = pb_decode(&stream, SimpleMessage_fields, &message);
	if (!status) {
		DEBUG_LOG("Decoding failed: %s\n", PB_GET_ERROR(&stream));
		return 1;
	}

	DEBUG_LOG("Your lucky number was %d!\n", (int)message.lucky_number);
	ExFreePool(config.data);

	return STATUS_FAILED_DRIVER_ENTRY;
}
