#include "file_mngr.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <iostream>
#include <fstream>
#include "rpc_log.h"

#define DATA_ROOT "data"

file_mngr *file_mngr::m_pthis = NULL;

/**
 * @brief construct
 */
file_mngr::file_mngr() {
    std::string path(DATA_ROOT);

    const char *chs = "0123456789abcdef";
    int chs_len = strlen(chs);

    mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWUSR| S_IXGRP;
    mkdir (path.c_str(), mode);
    for (int i = 0; i < chs_len; ++i) {
        for (int j = 0; j < chs_len; ++j) {
            mkdir((path + "/" + chs[i] + chs[j]).c_str(), mode);
        }
    }

    m_locks.resize(256);
}

/**
 * @brief destruct
 */
file_mngr::~file_mngr() {
}

/**
 * @brief create instance
 *
 * @return 
 */
file_mngr *file_mngr::create_instance() {
    if (!m_pthis) {
        m_pthis = new file_mngr;
    }
    return m_pthis;
}

/**
 * @brief destroy instance
 */
void file_mngr::destroy_instance() {
    if (m_pthis) {
        delete m_pthis;
    }
    m_pthis = NULL;
}

/**
 * @brief get file id
 *
 * @param file_id
 * @param file_content
 *
 * @return 
 */
int file_mngr::get_file_id(std::string &file_id, 
        const std::string &file_content) {

    unsigned char md5[16] = { 0 };
    MD5((const unsigned char*)file_content.c_str(), file_content.length(), md5);

    for (int i = 0; i < 16; ++i) {
        int low = md5[i] & 0x0f;
        int hig = (md5[i] & 0xf0) >> 4;
        if (hig < 10) {
            file_id += hig + '0';
        } else {
            file_id += hig - 10 + 'a';
        }
        if (low < 10) {
            file_id += low + '0';
        } else {
            file_id += low - 10 + 'a';
        }
    }
    return 0;
}

/**
 * @brief 
 *
 * @param file_id
 *
 * @return 
 */
int file_mngr::get_lock_index(const std::string &file_id) {
    char ch1 = file_id[0];
    char ch2 = file_id[1];
    return ch1 * 16 + ch2;
}

/**
 * @brief load file content
 *
 * @param file_id
 * @param file_content
 *
 * @return 
 */
int file_mngr::load(const std::string &file_id, 
        std::string &file_content) {

    int ret_val = -1;

    char path[128] = { 0 };
    sprintf(path, "%s/%s/%s", DATA_ROOT, 
            file_id.substr(0, 2).c_str(), file_id.c_str());

    /* read lock */
    auto_rdlock al(&m_locks[get_lock_index(file_id)]);

    std::ifstream in_file;
    in_file.open(path);
    if (in_file.is_open()) {
        in_file.seekg(0, in_file.end);
        file_content.resize(in_file.tellg());
        in_file.seekg(0, in_file.beg);

        in_file.read((char*)file_content.data(), file_content.size());
        in_file.close();
        ret_val = 0;
        RPC_DEBUG("save for file %s fail, path=%s", 
                file_id.c_str(), path);
    } else {
        RPC_DEBUG("retrieve for file %s fail, path=%s", 
                file_id.c_str(), path);
    }
    return ret_val;
}

/**
 * @brief save file content
 *
 * @param file_id
 * @param file_content
 *
 * @return 
 */
int file_mngr::save(const std::string &file_id, 
        const std::string &file_content) {

    int ret_val = -1;

    char path[128] = { 0 };
    sprintf(path, "%s/%s/%s", DATA_ROOT, 
            file_id.substr(0, 2).c_str(), file_id.c_str());

    /* write lock */
    auto_wrlock al(&m_locks[get_lock_index(file_id)]);

    std::ofstream out_file;
    out_file.open(path);
    if (out_file.is_open()) {
        out_file << file_content;
        out_file.close();
        ret_val = 0;
        RPC_DEBUG("save for file %s succ, path=%s", 
                file_id.c_str(), path);
    } else {
        RPC_DEBUG("save for file %s fail, path=%s", 
                file_id.c_str(), path);
    }
    return ret_val;
}
