from hikyuu.interactive.interactive import *
class TTT(Task):
    def __init__(self, x):
        super(TTT, self).__init__()
        self.x = x
        
    def run(self):
        print(self.x)
       
ts = []
tg = TaskGroup(2)
for i in range(5):
    tg.addTask(TTT(i))
tg.join()
