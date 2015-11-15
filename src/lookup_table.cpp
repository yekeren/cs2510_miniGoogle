#include "lookup_table.h"

/**
 * @brief construct
 */
lookup_table::lookup_table() {
}

/**
 * @brief destruct
 */
lookup_table::~lookup_table() {
}

/**
 * @brief lock group, return the single_table_t data structure
 *
 * @param group_id
 *
 * @return 
 */
const single_table_t &lookup_table::lock_group(int group_id) {
    rw_lock rwLock = m_locks.at(group_id);
    rwLock.wrlock();
    return m_tables.at(group_id);
}

/**
 * @brief unlock group
 *
 * @param group_id
 */
void lookup_table::unlock_group(int group_id) {
    rw_lock rwLock = m_locks.at(group_id);
    rwLock.unlock();
}

/**
 * @brief get file info
 *
 * @param file_id
 * @param file_info
 *
 * @return 
 */
int lookup_table::get_file_info(const std::string &file_id, 
        file_info_t &file_info) {
    int group_id = get_group_id(file_id);
    rw_lock rwLock = m_locks.at(group_id);
    rwLock.rdlock();
    single_table_t s_table = m_tables.at(group_id);
    single_table_t::iterator iter;
    int flag = 0;
    for(iter = s_table.begin();iter != s_table.end();iter ++){
        if(strcmp(file_id.c_str(), (iter->first).c_str()) == 0){
            file_info = iter->second;
            flag = 1;
            break;
        }
    }
    rwLock.unlock();
    if(flag == 0){
        return -1;
    }
    return 0;
}

/**
 * @brief set file info
 *
 * @param file_id
 * @param file_info
 *
 * @return 
 */
int lookup_table::set_file_info(const std::string &file_id, 
        const file_info_t &file_info) {
    int group_id = get_group_id(file_id);
    rw_lock rwLock = m_locks.at(group_id);
    rwLock.wrlock();
    single_table_t s_table = m_tables.at(group_id);
    s_table[file_id] = file_info;
    rwLock.unlock();
    return 0;
}

/**
 * @brief get group id by file_id
 *
 * @param file_id
 *
 * @return 
 */
int lookup_table::get_group_id(const std::string &file_id) {
    std::hash<std::string> hash_file;
    std::size_t file_ind = (hash_file(file_id)) % LOOKUP_TABLE_GROUP_NUM;
    return (int) file_ind;
}
