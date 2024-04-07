
#include <iostream>
#include <memory>

// 基类
class Base {
public:
    virtual void Show() const = 0;
    virtual ~Base() = default;
};

// 派生类A
class DerivedA : public Base {
public:
    void Show() const override {
        std::cout << "This is DerivedA." << std::endl;
    }
};

// 派生类B
class DerivedB : public Base {
public:
    void Show() const override {
        std::cout << "This is DerivedB." << std::endl;
    }
};

enum RestoreSceneType : int32_t {
    RESTORE_SCENE_TYPE_INVILID = -1, 
    RESTORE_SCENE_TYPE_SINGLE_CLONE, 
    RESTORE_SCENE_TYPE_DUAL_CLONE, 
    RESTORE_SCENE_TYPE_MAX, 
};

// 工厂类
class Factory {
public:
    static std::unique_ptr<Base> CreateObject(char type) {
        std::unique_ptr<Base> obj;
        switch (type) {
            case 'A':
                obj = std::make_unique<DerivedA>();
                break;
            case 'B':
                obj = std::make_unique<DerivedB>();
                break;
            // 可以根据需要添加更多类型的处理
            default:
                break;
        }
        return obj;
    }
};

int main() {
    // 使用工厂方法创建对象
    std::unique_ptr<Base> objA = Factory::CreateObject('A');
    std::unique_ptr<Base> objB = Factory::CreateObject('B');

    // 调用方法展示信息
    if (objA) objA->Show();
    if (objB) objB->Show();

    return 0;
}
