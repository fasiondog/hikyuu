from hikyuu.interactive.interactive import *
class TTT(Task):
    def __init__(self, x):
        self.x = x
        
    def run():
        print(self.x)
        
ts = []
tg = TaskGroup()
for i in range(100):
    ts.append(tg.addTask(TTT(i)))
tg.join()
