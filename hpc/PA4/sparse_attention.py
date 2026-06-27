import triton
import triton.language as tl


@triton.jit
def _sparse_indexer_64_kernel(
    q_idx,
    k_idx_cache,
    w_idx,
    block_table,
    context_lens,
    scores,
    MaxPages: tl.constexpr,
    PAGES_PER_PROG: tl.constexpr,
    BLOCK_M: tl.constexpr,
    BLOCK_D: tl.constexpr,
):
    page_group = tl.program_id(0) * PAGES_PER_PROG
    b = tl.program_id(1)

    offs_m = tl.arange(0, BLOCK_M)
    page_slot = offs_m // 64
    logical_page = page_group + page_slot
    row_in_page = offs_m - page_slot * 64
    seq = logical_page * 64 + row_in_page

    max_seq = MaxPages * 64
    visible = tl.load(context_lens + b)
    page_valid = logical_page < MaxPages
    row_valid = page_valid & (seq < visible)

    physical_page = tl.load(
        block_table + b * MaxPages + logical_page,
        mask=page_valid,
        other=0,
        eviction_policy="evict_last",
    )

    offs_h = tl.arange(0, 64)
    w = tl.load(
        w_idx + b * 64 + offs_h,
        eviction_policy="evict_last",
    ).to(tl.float32)

    acc = tl.zeros((BLOCK_M, 64), tl.float32)
    for d0 in tl.range(0, 128, BLOCK_D):
        offs_d = d0 + tl.arange(0, BLOCK_D)
        k = tl.load(
            k_idx_cache
            + ((physical_page * 64 + row_in_page)[:, None] * 128 + offs_d[None, :]),
            mask=row_valid[:, None],
            other=0.0,
            eviction_policy="evict_first",
        )
        q = tl.load(
            q_idx + b * 64 * 128 + offs_h[:, None] * 128 + offs_d[None, :],
            eviction_policy="evict_last",
        )
        acc += tl.dot(k, tl.trans(q), out_dtype=tl.float32)

    score = tl.sum(tl.maximum(acc, 0.0) * w[None, :], axis=1)
    out = tl.where(row_valid, score, -float("inf"))
    tl.store(scores + b * max_seq + seq, out, mask=page_valid)


@triton.jit
def _generic_indexer_kernel(
    q_idx,
    k_idx_cache,
    w_idx,
    block_table,
    context_lens,
    scores,
    Hidx: tl.constexpr,
    Didx: tl.constexpr,
    MaxPages: tl.constexpr,
    PageSize: tl.constexpr,
    BLOCK_M: tl.constexpr,
    BLOCK_H: tl.constexpr,
    BLOCK_D: tl.constexpr,
):
    pid_s = tl.program_id(0)
    b = tl.program_id(1)

    max_seq = MaxPages * PageSize
    offs_m = pid_s * BLOCK_M + tl.arange(0, BLOCK_M)
    valid_seq = offs_m < max_seq

    visible = tl.load(context_lens + b)
    logical_page = offs_m // PageSize
    page_offset = offs_m - logical_page * PageSize
    row_valid = valid_seq & (offs_m < visible)

    physical_page = tl.load(
        block_table + b * MaxPages + logical_page,
        mask=valid_seq,
        other=0,
    )

    offs_h = tl.arange(0, BLOCK_H)
    head_mask = offs_h < Hidx
    acc = tl.zeros((BLOCK_M, BLOCK_H), tl.float32)

    for d0 in tl.range(0, Didx, BLOCK_D):
        offs_d = d0 + tl.arange(0, BLOCK_D)
        d_mask = offs_d < Didx
        k = tl.load(
            k_idx_cache
            + ((physical_page * PageSize + page_offset)[:, None] * Didx + offs_d[None, :]),
            mask=row_valid[:, None] & d_mask[None, :],
            other=0.0,
        )
        q = tl.load(
            q_idx + b * Hidx * Didx + offs_h[:, None] * Didx + offs_d[None, :],
            mask=head_mask[:, None] & d_mask[None, :],
            other=0.0,
        )
        acc += tl.dot(k, tl.trans(q), out_dtype=tl.float32)

    w = tl.load(
        w_idx + b * Hidx + offs_h,
        mask=head_mask,
        other=0.0,
    ).to(tl.float32)
    score = tl.sum(tl.maximum(acc, 0.0) * w[None, :], axis=1)
    out = tl.where(row_valid, score, -float("inf"))
    tl.store(scores + b * max_seq + offs_m, out, mask=valid_seq)


def _pow2_at_least(x, floor):
    return max(floor, 1 << (x - 1).bit_length())


def run_kernel(
    q_idx,
    k_idx_cache,
    w_idx,
    block_table,
    context_lens,
    scores,
    B,
    Hidx,
    Didx,
    MaxPages,
    PageSize,
):
    if B <= 0 or MaxPages <= 0 or PageSize <= 0:
        return

    if Hidx == 64 and Didx == 128 and PageSize == 64:
        pages_per_prog = 2 if MaxPages >= 32 else 1
        grid = (triton.cdiv(MaxPages, pages_per_prog), B)
        _sparse_indexer_64_kernel[grid](
            q_idx,
            k_idx_cache,
            w_idx,
            block_table,
            context_lens,
            scores,
            MaxPages,
            PAGES_PER_PROG=pages_per_prog,
            BLOCK_M=pages_per_prog * 64,
            BLOCK_D=64,
            num_warps=4,
            num_stages=2,
        )
        return

    block_m = 16
    block_h = _pow2_at_least(Hidx, 16)
    block_d = 32
    grid = (triton.cdiv(MaxPages * PageSize, block_m), B)
    _generic_indexer_kernel[grid](
        q_idx,
        k_idx_cache,
        w_idx,
        block_table,
        context_lens,
        scores,
        Hidx,
        Didx,
        MaxPages,
        PageSize,
        BLOCK_M=block_m,
        BLOCK_H=block_h,
        BLOCK_D=block_d,
        num_warps=4,
        num_stages=2,
    )
