#!/usr/bin/env python

import workflow

opts = workflow.Options("pygsl_test")

job_mgr = workflow.Job_manager("pygsl_test.py", opts)
