/*!
    \file "Defs.cpp"

    Copyright (c) 2014 Matt Ervin / imp software (matt@impsoftware.org)
    Formatting: 120 columns, 4 spaces per tab, spaces only (no tab characters)
    Qt Coding Style: http://qt-project.org/wiki/Qt_Coding_Style
    Qt Coding Conventions: http://qt-project.org/wiki/Coding-Conventions
    Doc-tool: Doxygen (http://www.doxygen.com/)

    Application environment (and globals).

    ((( GNU General Public License Usage )))
    This file may be used under the terms of the GNU General Public License version 3.0 as published by
    the Free Software Foundation and appearing in the file LICENSE included in the packaging of this file.
    Please review the following information to ensure the GNU General Public License version 3.0 requirements
    will be met: http://www.gnu.org/copyleft/gpl.html.
*/


#include "Defs.hpp"
#include <errno.h>


uint
gatRqstToCode(GatRqst value)
{
    static uint const codes[] = {
        std::numeric_limits<uint>::max(), // EGatRqst::Undefined
        GatRqstCmd::SQ,                   // EGatRqst::01_SQ
        GatRqstCmd::LASQ,                 // EGatRqst::02_LASQ
        GatRqstCmd::LARQ,                 // EGatRqst::03_LARQ
        GatRqstCmd::IACQ,                 // EGatRqst::04_IACQ
    };
    Q_ASSERT(arycap(codes) == gatRqst_Count);
    Q_ASSERT(gatRqst_Count > static_cast<int>(value));
    return codes[static_cast<size_t>(value)];
}


GatRqst
gatRqstFromCode(uint value)
{
    static struct CodeMap {
        uint code_;
        GatRqst id_;
    } const codes[] = {
        { GatRqstCmd::SQ,   GatRqst::SQ_01, },
        { GatRqstCmd::LASQ, GatRqst::LASQ_02, },
        { GatRqstCmd::LARQ, GatRqst::LARQ_03, },
        { GatRqstCmd::IACQ, GatRqst::IACQ_04, },
    };
    for (size_t idx = 0; arycap(codes) > idx; ++idx)
    {
        if (codes[idx].code_ == value)
        {
            return codes[idx].id_;
        }
    }
    throw GatRqst::Undefined;
}


GatDataFormat
gatDataFormatCodeToId(uint code)
{
    return 0x01 == code ? GatDataFormat::PlainText :
           0x02 == code ? GatDataFormat::Xml :
           GatDataFormat::Undefined;
}


uint
gatDataFormatIdToCode(GatDataFormat gatDataFormat)
{
    return GatDataFormat::PlainText == gatDataFormat ? 0x01 :
           GatDataFormat::Xml == gatDataFormat ? 0x02 :
           0;
}


static uint16_t const crc16Table[] =
{
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040,
};


uint16_t
calcGatCrc16(void const *data, size_t dataSize, uint16_t seed)
{
    uint8_t const *bytes = reinterpret_cast<uint8_t const *>(data);
    uint16_t crc = seed;
    for (size_t idx = 0; dataSize > idx; ++idx)
    {
        crc = static_cast<uint16_t>((crc >> 8) ^ crc16Table[(crc ^ (static_cast<uint16_t>(bytes[idx]) & 0xff)) & 0xff]);
    }
    return crc;
}


static struct ErrnoDesc {
    int errorno_;
    char const *description_;
} errnoDescs[] = {
    { EPERM, "EPERM: Operation not permitted" },
    { ENOENT, "ENOENT: No such file or directory" },
    { ESRCH, "ESRCH: No such process" },
    { EINTR, "EINTR: Interrupted system call" },
    { EIO, "EIO: I/O error" },
    { ENXIO, "ENXIO: No such device or address" },
    { E2BIG, "E2BIG: Argument list too long" },
    { ENOEXEC, "ENOEXEC: Exec format error" },
    { EBADF, "EBADF: Bad file number" },
    { ECHILD, "ECHILD: No child processes" },
    { EAGAIN, "EAGAIN: Try again" },
    { ENOMEM, "ENOMEM: Out of memory" },
    { EACCES, "EACCES: Permission denied" },
    { EFAULT, "EFAULT: Bad address" },
    { ENOTBLK, "ENOTBLK: Block device required" },
    { EBUSY, "EBUSY: Device or resource busy" },
    { EEXIST, "EEXIST: File exists" },
    { EXDEV, "EXDEV: Cross-device link" },
    { ENODEV, "ENODEV: No such device" },
    { ENOTDIR, "ENOTDIR: Not a directory" },
    { EISDIR, "EISDIR: Is a directory" },
    { EINVAL, "EINVAL: Invalid argument" },
    { ENFILE, "ENFILE: File table overflow" },
    { EMFILE, "EMFILE: Too many open files" },
    { ENOTTY, "ENOTTY: Not a typewriter" },
    { ETXTBSY, "ETXTBSY: Text file busy" },
    { EFBIG, "EFBIG: File too large" },
    { ENOSPC, "ENOSPC: No space left on device" },
    { ESPIPE, "ESPIPE: Illegal seek" },
    { EROFS, "EROFS: Read-only file system" },
    { EMLINK, "EMLINK: Too many links" },
    { EPIPE, "EPIPE: Broken pipe" },
    { EDOM, "EDOM: Math argument out of domain of func" },
    { ERANGE, "ERANGE: Math result not representable" },
    { EDEADLK, "EDEADLK: Resource deadlock would occur" },
    { ENAMETOOLONG, "ENAMETOOLONG: File name too long" },
    { ENOLCK, "ENOLCK: No record locks available" },
    { ENOSYS, "ENOSYS: Function not implemented" },
    { ENOTEMPTY, "ENOTEMPTY: Directory not empty" },
    { ELOOP, "ELOOP: Too many symbolic links encountered" },
    { ENOMSG, "ENOMSG: No message of desired type" },
    { EIDRM, "EIDRM: Identifier removed" },
    { ECHRNG, "ECHRNG: Channel number out of range" },
    { EL2NSYNC, "EL2NSYNC: Level 2 not synchronized" },
    { EL3HLT, "EL3HLT: Level 3 halted" },
    { EL3RST, "EL3RST: Level 3 reset" },
    { ELNRNG, "ELNRNG: Link number out of range" },
    { EUNATCH, "EUNATCH: Protocol driver not attached" },
    { ENOCSI, "ENOCSI: No CSI structure available" },
    { EL2HLT, "EL2HLT: Level 2 halted" },
    { EBADE, "EBADE: Invalid exchange" },
    { EBADR, "EBADR: Invalid request descriptor" },
    { EXFULL, "EXFULL: Exchange full" },
    { ENOANO, "ENOANO: No anode" },
    { EBADRQC, "EBADRQC: Invalid request code" },
    { EBADSLT, "EBADSLT: Invalid slot" },
    { EBFONT, "EBFONT: Bad font file format" },
    { ENOSTR, "ENOSTR: Device not a stream" },
    { ENODATA, "ENODATA: No data available" },
    { ETIME, "ETIME: Timer expired" },
    { ENOSR, "ENOSR: Out of streams resources" },
    { ENONET, "ENONET: Machine is not on the network" },
    { ENOPKG, "ENOPKG: Package not installed" },
    { EREMOTE, "EREMOTE: Object is remote" },
    { ENOLINK, "ENOLINK: Link has been severed" },
    { EADV, "EADV: Advertise error" },
    { ESRMNT, "ESRMNT: Srmount error" },
    { ECOMM, "ECOMM: Communication error on send" },
    { EPROTO, "EPROTO: Protocol error" },
    { EMULTIHOP, "EMULTIHOP: Multihop attempted" },
    { EDOTDOT, "EDOTDOT: RFS specific error" },
    { EBADMSG, "EBADMSG: Not a data message" },
    { EOVERFLOW, "EOVERFLOW: Value too large for defined data type" },
    { ENOTUNIQ, "ENOTUNIQ: Name not unique on network" },
    { EBADFD, "EBADFD: File descriptor in bad state" },
    { EREMCHG, "EREMCHG: Remote address changed" },
    { ELIBACC, "ELIBACC: Can not access a needed shared library" },
    { ELIBBAD, "ELIBBAD: Accessing a corrupted shared library" },
    { ELIBSCN, "ELIBSCN: .lib section in a.out corrupted" },
    { ELIBMAX, "ELIBMAX: Attempting to link in too many shared libraries" },
    { ELIBEXEC, "ELIBEXEC: Cannot exec a shared library directly" },
    { EILSEQ, "EILSEQ: Illegal byte sequence" },
    { ERESTART, "ERESTART: Interrupted system call should be restarted" },
    { ESTRPIPE, "ESTRPIPE: Streams pipe error" },
    { EUSERS, "EUSERS: Too many users" },
    { ENOTSOCK, "ENOTSOCK: Socket operation on non-socket" },
    { EDESTADDRREQ, "EDESTADDRREQ: Destination address required" },
    { EMSGSIZE, "EMSGSIZE: Message too long" },
    { EPROTOTYPE, "EPROTOTYPE: Protocol wrong type for socket" },
    { ENOPROTOOPT, "ENOPROTOOPT: Protocol not available" },
    { EPROTONOSUPPORT, "EPROTONOSUPPORT: Protocol not supported" },
    { ESOCKTNOSUPPORT, "ESOCKTNOSUPPORT: Socket type not supported" },
    { EOPNOTSUPP, "EOPNOTSUPP: Operation not supported on transport endpoint" },
    { EPFNOSUPPORT, "EPFNOSUPPORT: Protocol family not supported" },
    { EAFNOSUPPORT, "EAFNOSUPPORT: Address family not supported by protocol" },
    { EADDRINUSE, "EADDRINUSE: Address already in use" },
    { EADDRNOTAVAIL, "EADDRNOTAVAIL: Cannot assign requested address" },
    { ENETDOWN, "ENETDOWN: Network is down" },
    { ENETUNREACH, "ENETUNREACH: Network is unreachable" },
    { ENETRESET, "ENETRESET: Network dropped connection because of reset" },
    { ECONNABORTED, "ECONNABORTED: Software caused connection abort" },
    { ECONNRESET, "ECONNRESET: Connection reset by peer" },
    { ENOBUFS, "ENOBUFS: No buffer space available" },
    { EISCONN, "EISCONN: Transport endpoint is already connected" },
    { ENOTCONN, "ENOTCONN: Transport endpoint is not connected" },
    { ESHUTDOWN, "ESHUTDOWN: Cannot send after transport endpoint shutdown" },
    { ETOOMANYREFS, "ETOOMANYREFS: Too many references: cannot splice" },
    { ETIMEDOUT, "ETIMEDOUT: Connection timed out" },
    { ECONNREFUSED, "ECONNREFUSED: Connection refused" },
    { EHOSTDOWN, "EHOSTDOWN: Host is down" },
    { EHOSTUNREACH, "EHOSTUNREACH: No route to host" },
    { EALREADY, "EALREADY: Operation already in progress" },
    { EINPROGRESS, "EINPROGRESS: Operation now in progress" },
    { ESTALE, "ESTALE: Stale NFS file handle" },
    { EUCLEAN, "EUCLEAN: Structure needs cleaning" },
    { ENOTNAM, "ENOTNAM: Not a XENIX named type file" },
    { ENAVAIL, "ENAVAIL: No XENIX semaphores available" },
    { EISNAM, "EISNAM: Is a named type file" },
    { EREMOTEIO, "EREMOTEIO: Remote I/O error" },
    { EDQUOT, "EDQUOT: Quota exceeded" },
    { ENOMEDIUM, "ENOMEDIUM: No medium found" },
    { EMEDIUMTYPE, "EMEDIUMTYPE: Wrong medium type" },
    { ECANCELED, "ECANCELED: Operation Canceled" },
    { ENOKEY, "ENOKEY: Required key not available" },
    { EKEYEXPIRED, "EKEYEXPIRED: Key has expired" },
    { EKEYREVOKED, "EKEYREVOKED: Key has been revoked" },
    { EKEYREJECTED, "EKEYREJECTED: Key was rejected by service" },
    { EOWNERDEAD, "EOWNERDEAD: Owner died" },
    { ENOTRECOVERABLE, "ENOTRECOVERABLE: State not recoverable" },
    { ERFKILL, "ERFKILL: Operation not possible due to RF-kill" },
    { EHWPOISON, "EHWPOISON: Memory page has hardware error" },
};


char const *
errnoToStr(int errorNumber)
{
    for (ErrnoDesc const &desc : errnoDescs)
    {
        if (desc.errorno_ == errorNumber)
        {
            return desc.description_;
        }
    }

    return "";
}


/*
    End of "Defs.cpp"
*/
