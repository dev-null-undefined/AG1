#include <iostream>
#include <memory>
#include <fstream>
#include "avl_tree_v2.hpp"

auto main() -> signed {

    auto node = std::make_shared<AugmentedNode>();
    node->setValue(4);
    node->generateGraph();
//    node.find<&FilteredSizeCounter<char, '\n'>::Inner<NodeType>::size>(0);
    auto t = node->find<&SizeCounter<AugmentedNode>::size>(0);
    std::cout << t.getValue() << std::endl;

    std::shared_ptr<AugmentedNode> node2 = std::make_shared<AugmentedNode>();
    node->setChild<Direction::right>(node2)->setValue(99);

    node->right->setChild(Direction::right, std::make_shared<AugmentedNode>())->setValue(98);
    node->right->right->setChild(Direction::right, std::make_shared<AugmentedNode>())->setValue(97);

//    node = node->insert(std::make_shared<NodeType>()).setValue(42).shared_from_this();

    system("rm -rf file*.dot file*.png");
    for (int i = 0; i < 8; ++i) {
        if (i != 0)
            node->right->insert(std::make_shared<AugmentedNode>()).setValue(i);
        if (i == 3 || i == 7) {
            std::ofstream fs("file" + std::to_string(i) + "-d.dot");
            node->generateGraph(fs);
            fs.close();
            system(("dot -T png < file" + std::to_string(i) + "-d.dot > image" + std::to_string(i) + "-d.png").c_str());
            node->right->remove(node);
        }

        if (i == 4) {
            std::ofstream fs("file" + std::to_string(i) + "-r.dot");
            node->generateGraph(fs);
            fs.close();
            system(("dot -T png < file" + std::to_string(i) + "-r.dot > image" + std::to_string(i) + "-r.png").c_str());
            node->right->rotateLeft();
        }

        std::ofstream fs("file" + std::to_string(i) + ".dot");
        node->generateGraph(fs);
        fs.close();
        system(("dot -T png < file" + std::to_string(i) + ".dot > image" + std::to_string(i) + ".png").c_str());
    }

    for (int i = 0; i < 7; ++i) {
        std::cout << int(node->find<&SizeCounter<AugmentedNode>::size>(i).getValue()) << std::endl;
    }
    return 0;
}