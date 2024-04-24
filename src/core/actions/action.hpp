
namespace core::actions {

  class Action {
    virtual void redo();

    virtual void undo();
  };
}  // namespace core::actions