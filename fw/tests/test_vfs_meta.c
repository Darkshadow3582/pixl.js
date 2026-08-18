#include "test_framework.h"
#include "vfs.h"
#include "vfs_meta.h"

static void test_encode_decode_roundtrip(void) {
    TF_CASE("meta encode/decode roundtrip with all fields");
    uint8_t meta[VFS_META_MAX_SIZE + 1];
    memset(meta, 0, sizeof(meta));

    vfs_meta_t in;
    memset(&in, 0, sizeof(in));
    in.has_notes = true;
    strcpy(in.notes, "hello world");
    in.has_flags = true;
    in.flags = 0x5A;
    in.has_amiibo_id = true;
    in.amiibo_head = 0x01931001;
    in.amiibo_tail = 0x1B3B3C1;

    vfs_meta_encode(meta, sizeof(meta), &in);

    vfs_meta_t out;
    memset(&out, 0, sizeof(out));
    vfs_meta_decode(meta, sizeof(meta), &out);

    TF_CHECK(out.has_notes);
    TF_CHECK(strcmp(out.notes, "hello world") == 0);
    TF_CHECK(out.has_flags);
    TF_CHECK_EQ(out.flags, 0x5A);
    TF_CHECK(out.has_amiibo_id);
    TF_CHECK_EQ(out.amiibo_head, 0x01931001);
    TF_CHECK_EQ(out.amiibo_tail, 0x1B3B3C1);
}

static void test_decode_empty(void) {
    TF_CASE("decode of empty/unset meta keeps output clean");
    uint8_t meta[128];
    memset(meta, 0, sizeof(meta));

    vfs_meta_t out;
    memset(&out, 0, sizeof(out));
    vfs_meta_decode(meta, sizeof(meta), &out);
    TF_CHECK(!out.has_notes);
    TF_CHECK(!out.has_flags);
    TF_CHECK(!out.has_amiibo_id);

    // 0xFF also means "unset"
    memset(meta, 0xFF, sizeof(meta));
    vfs_meta_decode(meta, sizeof(meta), &out);
    TF_CHECK(!out.has_notes);
    TF_CHECK(!out.has_flags);
    TF_CHECK(!out.has_amiibo_id);
}

static void test_partial_fields(void) {
    TF_CASE("encode/decode with only flags set");
    uint8_t meta[VFS_META_MAX_SIZE + 1];
    memset(meta, 0, sizeof(meta));

    vfs_meta_t in;
    memset(&in, 0, sizeof(in));
    in.has_flags = true;
    in.flags = VFS_OBJ_FLAG_READONLY | VFS_OBJ_FLAG_HIDDEN;

    vfs_meta_encode(meta, sizeof(meta), &in);

    vfs_meta_t out;
    memset(&out, 0, sizeof(out));
    vfs_meta_decode(meta, sizeof(meta), &out);

    TF_CHECK(!out.has_notes);
    TF_CHECK(!out.has_amiibo_id);
    TF_CHECK(out.has_flags);
    TF_CHECK_EQ(out.flags, VFS_OBJ_FLAG_READONLY | VFS_OBJ_FLAG_HIDDEN);
}

int main(void) {
    test_encode_decode_roundtrip();
    test_decode_empty();
    test_partial_fields();
    TF_MAIN_END();
}
