MAX = 3

class Node():
    def __init__(self, value):
        global MAX
        self.value = value
        if(MAX > 0):
            MAX -= 1
            self.nodes = [Node(value + 'A'), Node(value + 'C'),Node(value + 'G'),Node(value + 'T')]
        else:
            self.nodes = None

    def print_line(self):
        if self.nodes != None:
            for node in self.nodes:
                print(node.value)
                print(node.print_line())


master = Node('')
master.print_line()
