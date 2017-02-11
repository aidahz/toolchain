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
#include <aws/codepipeline/CodePipeline_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/codepipeline/model/JobData.h>

namespace Aws
{
namespace Utils
{
namespace Json
{
  class JsonValue;
} // namespace Json
} // namespace Utils
namespace CodePipeline
{
namespace Model
{

  /**
   * <p>Represents information about the details of a job.</p>
   */
  class AWS_CODEPIPELINE_API JobDetails
  {
  public:
    JobDetails();
    JobDetails(const Aws::Utils::Json::JsonValue& jsonValue);
    JobDetails& operator=(const Aws::Utils::Json::JsonValue& jsonValue);
    Aws::Utils::Json::JsonValue Jsonize() const;

    /**
     * <p>The unique system-generated ID of the job.</p>
     */
    inline const Aws::String& GetId() const{ return m_id; }

    /**
     * <p>The unique system-generated ID of the job.</p>
     */
    inline void SetId(const Aws::String& value) { m_idHasBeenSet = true; m_id = value; }

    /**
     * <p>The unique system-generated ID of the job.</p>
     */
    inline void SetId(Aws::String&& value) { m_idHasBeenSet = true; m_id = value; }

    /**
     * <p>The unique system-generated ID of the job.</p>
     */
    inline void SetId(const char* value) { m_idHasBeenSet = true; m_id.assign(value); }

    /**
     * <p>The unique system-generated ID of the job.</p>
     */
    inline JobDetails& WithId(const Aws::String& value) { SetId(value); return *this;}

    /**
     * <p>The unique system-generated ID of the job.</p>
     */
    inline JobDetails& WithId(Aws::String&& value) { SetId(value); return *this;}

    /**
     * <p>The unique system-generated ID of the job.</p>
     */
    inline JobDetails& WithId(const char* value) { SetId(value); return *this;}

    
    inline const JobData& GetData() const{ return m_data; }

    
    inline void SetData(const JobData& value) { m_dataHasBeenSet = true; m_data = value; }

    
    inline void SetData(JobData&& value) { m_dataHasBeenSet = true; m_data = value; }

    
    inline JobDetails& WithData(const JobData& value) { SetData(value); return *this;}

    
    inline JobDetails& WithData(JobData&& value) { SetData(value); return *this;}

    /**
     * <p>The AWS account ID associated with the job.</p>
     */
    inline const Aws::String& GetAccountId() const{ return m_accountId; }

    /**
     * <p>The AWS account ID associated with the job.</p>
     */
    inline void SetAccountId(const Aws::String& value) { m_accountIdHasBeenSet = true; m_accountId = value; }

    /**
     * <p>The AWS account ID associated with the job.</p>
     */
    inline void SetAccountId(Aws::String&& value) { m_accountIdHasBeenSet = true; m_accountId = value; }

    /**
     * <p>The AWS account ID associated with the job.</p>
     */
    inline void SetAccountId(const char* value) { m_accountIdHasBeenSet = true; m_accountId.assign(value); }

    /**
     * <p>The AWS account ID associated with the job.</p>
     */
    inline JobDetails& WithAccountId(const Aws::String& value) { SetAccountId(value); return *this;}

    /**
     * <p>The AWS account ID associated with the job.</p>
     */
    inline JobDetails& WithAccountId(Aws::String&& value) { SetAccountId(value); return *this;}

    /**
     * <p>The AWS account ID associated with the job.</p>
     */
    inline JobDetails& WithAccountId(const char* value) { SetAccountId(value); return *this;}

  private:
    Aws::String m_id;
    bool m_idHasBeenSet;
    JobData m_data;
    bool m_dataHasBeenSet;
    Aws::String m_accountId;
    bool m_accountIdHasBeenSet;
  };

} // namespace Model
} // namespace CodePipeline
} // namespace Aws