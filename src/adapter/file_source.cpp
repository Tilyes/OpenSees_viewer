// =============================================================================
// 文件: adapter/file_source.cpp
// 作用: FileSource 实现：TCL 解析器，识别 node/element 命令生成 Mesh。调用链: App (菜单) → FileSource::load → Model
// 依赖: viewer_core (Mesh, Model, Field)
// =============================================================================

#include "file_source.h"
#include <spdlog/spdlog.h>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cctype>


namespace viewer {
	// 按空格分词，返回一个字符串列表
    std::vector<std::string> tokenize(const std::string& line) {
        std::vector<std::string> tokens;  // 大框
        std::string word;                 // 篮子

        for (char c : line) {             // 遍历每个字符
            if (isspace(c)) {             // 是空格？
                if (!word.empty()) {      // 篮子里有东西？
                    tokens.push_back(word);  // 倒进大框
                    word.clear();            // 清空篮子
                }
            }
            else {
                word += c;                // 放到篮子
            }
        }
        // 最后如果篮子里还有东西
        if (!word.empty()) {
            tokens.push_back(word);
        }
        return tokens;
    }

    bool FileSource::load(const std::string& path, Model& out) {
        std::ifstream file(path);
        if (!file.is_open()) {
            spdlog::error("FileSource: cannot open {}", path);
            return false;
        };

        Mesh mesh;

        std::string line;
        while (std::getline(file, line)) {
            // 跳过空行和注释行
            if (line.empty())  continue;
            if (line[0] == '#') continue;

            auto tokens = tokenize(line);
            if (tokens[0] == "node") {
                int nodeid = std::stoi(tokens[1]);
                float x = std::stof(tokens[2]);
                float y = std::stof(tokens[3]);
                mesh.add_node(nodeid, glm::vec3(x, y, 0));
            };

            if (tokens[0] == "element") {
                std::string eletype = std::move(tokens[1]);
                int eleid = std::stoi(tokens[2]);
                uint32_t enode1 = static_cast<uint32_t>(std::stoi(tokens[3]));
                uint32_t enode2 = static_cast<uint32_t>(std::stoi(tokens[4]));

                if (eletype == "forceBeamColumn" || eletype == "truss" || eletype == "zeroLength") {
                    mesh.add_element(eleid, ElementType::Line2, { enode1, enode2 });
                };
            };
        };
        spdlog::info("Loaded {} nodes, {} elements", mesh.nodes().size(), mesh.elements().size());
        out.set_mesh(std::move(mesh));
        return true;
    }

} // namespace viewer
