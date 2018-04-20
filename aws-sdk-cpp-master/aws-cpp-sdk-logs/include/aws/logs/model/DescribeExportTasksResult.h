﻿/*
* Copyright 2010-2016 Amazon.com, Inc. or its affiliates. All Rights Reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License").
* You may not use this file except in compliance with the License.
* A copy of the License is located at
*
*  http://aws.amazon.com/apache2.0
*
* or in the "license" file accompanying this file. This file is distributed
* on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
* express or implied. See the License for the specific language governing
* permissions and limitations under the License.
*/
#pragma once
#include <aws/logs/CloudWatchLogs_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSVector.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/logs/model/ExportTask.h>

namespace Aws
{
template<typename RESULT_TYPE>
class AmazonWebServiceResult;

namespace Utils
{
namespace Json
{
  class JsonValue;
} // namespace Json
} // namespace Utils
namespace CloudWatchLogs
{
namespace Model
{
  class AWS_CLOUDWATCHLOGS_API DescribeExportTasksResult
  {
  public:
    DescribeExportTasksResult();
    DescribeExportTasksResult(const AmazonWebServiceResult<Aws::Utils::Json::JsonValue>& result);
    DescribeExportTasksResult& operator=(const AmazonWebServiceResult<Aws::Utils::Json::JsonValue>& result);

    
    inline const Aws::Vector<ExportTask>& GetExportTasks() const{ return m_exportTasks; }

    
    inline void SetExportTasks(const Aws::Vector<ExportTask>& value) { m_exportTasks = value; }

    
    inline void SetExportTasks(Aws::Vector<ExportTask>&& value) { m_exportTasks = value; }

    
    inline DescribeExportTasksResult& WithExportTasks(const Aws::Vector<ExportTask>& value) { SetExportTasks(value); return *this;}

    
    inline DescribeExportTasksResult& WithExportTasks(Aws::Vector<ExportTask>&& value) { SetExportTasks(value); return *this;}

    
    inline DescribeExportTasksResult& AddExportTasks(const ExportTask& value) { m_exportTasks.push_back(value); return *this; }

    
    inline DescribeExportTasksResult& AddExportTasks(ExportTask&& value) { m_exportTasks.push_back(value); return *this; }

    
    inline const Aws::String& GetNextToken() const{ return m_nextToken; }

    
    inline void SetNextToken(const Aws::String& value) { m_nextToken = value; }

    
    inline void SetNextToken(Aws::String&& value) { m_nextToken = value; }

    
    inline void SetNextToken(const char* value) { m_nextToken.assign(value); }

    
    inline DescribeExportTasksResult& WithNextToken(const Aws::String& value) { SetNextToken(value); return *this;}

    
    inline DescribeExportTasksResult& WithNextToken(Aws::String&& value) { SetNextToken(value); return *this;}

    
    inline DescribeExportTasksResult& WithNextToken(const char* value) { SetNextToken(value); return *this;}

  private:
    Aws::Vector<ExportTask> m_exportTasks;
    Aws::String m_nextToken;
  };

} // namespace Model
} // namespace CloudWatchLogs
} // namespace Aws