## 算法思路和实现要点

最终采用 **FHQ Treap** 完成题目  
核心思想是把舞者序列视为一棵按中序表示序列的平衡树，节点中不存显式 key，位置由左子树大小隐式决定    

与 splay tree 相比，FHQ Treap 通过 `split/merge` 组合区间操作，常数更小，代码实现也更简单    

代码主要有以下函数：  
1. `newNode(value)`：创建新节点，初始化左右子节点、子树大小、标签、随机优先级。随机优先级由 `nextRandom()` 产生，用于维持 Treap 的堆性质，保证期望高度 `O(log n)`
2. `pushUp(node)`：维护 `sub_size`，`size = size(left) + size(right) + 1`，每次 `split/merge` 回溯时调用
3. `applyReverse(node)` 与 `pushDown(node)`：`applyReverse` 只交换左右儿子并翻转 `reverse_tag`，不立刻下探整段区间；`pushDown` 在访问节点时把翻转标记传到子节点，配合起到一个懒加载的效果
4. `split(node, k, left_tree, right_tree)`：按前 k 个节点切树，返回两棵树，左树包含前 k 个元素，右树包含其余元素
5. `merge(left_tree, right_tree)`：合并两棵有序的树，按随机优先级选根并递归拼接，回溯时 `pushUp`
6. `kthNode(node, k)`：按子树大小迭代下降定位第 `k`（0-based）个节点
7. `opI(label, pos)`：先通过 `split(root, pos)` 拆成 `A|B`，再 `merge(A, new, B)`，达成插入的效果
8. `opS(i, j)`：用 `kthNode` 找两节点并交换 `label`，而不改树结构。
9. `opR(r, current_size)`：归一化 `r` 到 `[0, n)` 后，切成 `A(前 n-r)` 与 `B(后 r)`，重组为 `B+A`
10. `reverseRange(l, r)` 与 `opF(i, j)`：`reverseRange` 通过两次 split 分离 `[l,r]`，对中段打翻转标记再 merge 回去
11. `opF` 分为两种情形：  
    - `i<=j`：直接 `reverseRange(i,j)`；  
    - `i>j`：按循环区间定义，通过旋转、反转再旋转的操作达成同样效果
12. `inOrder`：中序遍历保存结果，然后输出

## 渐进时间复杂度的分析，包含过程

设最终舞者数为 `N`，操作数为 `M`，且 `N<=M`。Treap 期望高度为 `O(log N)`  
则各函数时间复杂度分析如下：
1. `split` / `merge`：每次递归沿树高走一条链，期望访问节点数为 `O(log N)`
2. `opI`：1 次 `split` + 2 次 `merge` + 常数次维护，期望 `O(log N)`
3. `opS`：2 次 `kthNode`，每次沿树高下降，期望 `O(log N)`，交换值为 `O(1)`，总计 `O(log N)`
4. `opR`：1 次 `split` + 1 次 `merge`，期望 `O(log N)`。
5. `reverseRange`：2 次 `split` + 2 次 `merge` + 1 次打标记，期望 `O(log N)`
6. `opF`：
   - `i<=j`：1 次 `reverseRange`，期望 `O(log N)`；  
   - `i>j`：2 次 `opR` + 1 次 `reverseRange`，仍是常数个 `split/merge` 组合，期望 `O(log N)`。
   - 
因此每个操作期望时间均为 `O(log N)`，总时间复杂度为：$T(M,N)=O(M\log N)$

输出阶段中序遍历 `N` 个节点，需 `O(N)`。整体为：$O(M\log N + N)$

在 `M` 主导时为 $O(M\log N)$

## 渐进空间复杂度的分析，包含过程

1. 节点池 `tree[MAX_NODES]`: 每个舞者对应 1 个节点，最终节点数 `N`，占用 `O(N)`
2. 递归调用栈：`split/merge/inOrder` 的递归深度与树高一致，期望 `O(log N)`。
3. res 数组：`inOrder` 收集最终答案 `N` 个整数，为 `O(N)`

因此渐进空间复杂度为：$O(N)$

## 遇到的困难和解决方法

最初采用的 splay tree 总是无法通过部分测例，猜测是在某种极端情况下发生了退化，但始终没能找到更好的解决办法  
后来改用 FHQ Treap，通过 `split/merge` 进行区间操作，降低实现复杂度的常数，省去维护平衡树时大量的旋转操作

## 估计完成本题所用时间 

6 小时

## 关于本题的更多感想

虽然理论上理解和使用 splay tree 能够很轻松的解决问题，但在特殊情况下发生的退化却始料不及，学习的路还很长...

不过新学习的无需旋转的平衡树 FHQ treap 确实很让人惊喜，在实现上并不复杂，但表现出极强的稳定性和不错的性能 