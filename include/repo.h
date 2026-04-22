#ifndef REPO_H
#define REPO_H

#include <cjson/cJSON.h>

// ---------- File loading / parsing ----------
char* read_file(const char* path);

// Parse repo JSON file into a cJSON tree.
// Caller must call cJSON_Delete(root).
cJSON* repo_get_parsed_tree(const char* path);

// ---------- Version selection ----------
int compare_versions(const char* v1, const char* v2);

// Given a package object (packages["name"]), return the latest version NODE
// from pkg["versions"].
// The returned cJSON* is a node whose key is the version string (ver->string).
cJSON* repo_get_latest_version(cJSON* pkg);

// ---------- Package metadata helpers for new schema ----------

// Get the "packages" object from the root JSON:
// root["packages"].
cJSON* repo_get_packages_root(cJSON* root);

// Get a package object by name from the packages root:
// packages["pkgname"].
cJSON* repo_get_package(cJSON* packages_root, const char* pkgname);

// Get the versions object from a package:
// pkg["versions"].
cJSON* repo_get_versions(cJSON* pkg);

// Get a specific version node by version string:
// pkg["versions"][version_str].
cJSON* repo_get_version_node(cJSON* pkg, const char* version_str);

// Get dependencies object from a version node:
// version_node["dependencies"].
cJSON* repo_get_dependencies(cJSON* version_node);

// ---------- GitHub tags-only support ----------

// Extract GitHub owner/repo from pkg["source"].
// Returns 1 on success, 0 on failure.
// Returned strings are owned by cJSON; do not free.
int repo_get_github_owner_repo(cJSON* pkg,
                               const char** out_owner,
                               const char** out_repo);

// Extract the tag name from version_node["ref"], verifying ref.type == "tag".
// Returns 1 on success, 0 on failure.
// Returned string is owned by cJSON; do not free.
int repo_get_tag_name(cJSON* version_node,
                      const char** out_tag);

// Build the tarball URL for a GitHub tag.
// Returns malloc()'d string you must free().
char* repo_build_github_tag_tarball_url(const char* owner,
                                        const char* repo,
                                        const char* tag);

// ---------- Legacy URL accessor (if you still keep "url" somewhere) ----------
// If your new schema does NOT include "url", you can remove this.

#endif