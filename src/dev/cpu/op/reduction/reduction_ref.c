/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * License); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * AS IS BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*
 * Copyright (c) 2020, OPEN AI LAB
 * Author: zpeng@openailab.com
 */

#include "sys_port.h"
#include "module.h"
#include "tengine_errno.h"
#include "tengine_log.h"
#include "tengine_ir.h"
#include "../../cpu_node_ops.h"
#include "tengine_op.h"
#include "reduction_param.h"
#include <math.h>
#include "reduction_kernel_ref.h"

static int init_node(struct node_ops* node_ops, struct exec_node* exec_node, struct exec_graph* exec_graph)
{
    return 0;
}

static int release_node(struct node_ops* node_ops, struct exec_node* exec_node, struct exec_graph* exec_graph)
{
    return 0;
}

static int prerun(struct node_ops* node_ops, struct exec_node* exec_node, struct exec_graph* exec_graph)
{
    return 0;
}

static int run(struct node_ops* node_ops, struct exec_node* exec_node, struct exec_graph* exec_graph)
{
    struct ir_node* ir_node = exec_node->ir_node;
    struct ir_graph* ir_graph = ir_node->graph;
    struct ir_tensor* input_tensor;
    struct ir_tensor* output_tensor;

    input_tensor = get_ir_graph_tensor(ir_graph, ir_node->input_tensors[0]);
    output_tensor = get_ir_graph_tensor(ir_graph, ir_node->output_tensors[0]);
    struct reduction_param* reduction_param = ( struct reduction_param* )ir_node->op.param_mem;
    struct reduce_param_ref param;
    int out_tensor_size = 1;
    for (int i = 0; i < output_tensor->dim_num; i++)
    {
        out_tensor_size *= output_tensor->dims[i];
    }
    int element_size = output_tensor->elem_size;

    int dims[4] = {1, 1, 1, 1};

    for (int i = 0; i < input_tensor->dim_num; i++)
    {
        dims[i] = input_tensor->dims[i];
    }
    int dim0 = dims[0];
    int dim1 = dims[1];
    int dim2 = dims[2];
    int dim3 = dims[3];

    param.param_dim[0] = reduction_param->dim_0;
    param.param_dim[1] = reduction_param->dim_1;
    param.param_dim[2] = reduction_param->dim_2;
    param.param_dim[3] = reduction_param->dim_3;
    param.type = reduction_param->type;

    int ret = ref_reduce_fp32(( float* )input_tensor->data, ( float* )output_tensor->data, dim0, dim1, dim2, dim3,
                              out_tensor_size, &param);
    if (ret < 0)
        return -1;
    else
        return 0;
}

static int score(struct node_ops* node_ops, struct exec_graph* exec_graph, struct ir_node* exec_node)
{
    return OPS_SCORE_CANDO;
}

static struct node_ops hcl_node_ops = {.prerun = prerun,
                                       .run = run,
                                       .reshape = NULL,
                                       .postrun = NULL,
                                       .init_node = init_node,
                                       .release_node = release_node,
                                       .score = score};

static int reg_reduction_hcl_ops(void* arg)
{
    return register_builtin_node_ops(OP_REDUCTION, &hcl_node_ops);
}

static int unreg_reduction_hcl_ops(void* arg)
{
    return unregister_builtin_node_ops(OP_REDUCTION, &hcl_node_ops);
}

AUTO_REGISTER_OPS(reg_reduction_hcl_ops);
AUTO_UNREGISTER_OPS(unreg_reduction_hcl_ops);