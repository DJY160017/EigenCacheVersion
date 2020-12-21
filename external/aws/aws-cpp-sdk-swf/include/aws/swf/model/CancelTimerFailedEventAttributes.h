﻿/*
* Copyright 2010-2017 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <aws/swf/SWF_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/swf/model/CancelTimerFailedCause.h>
#include <utility>

namespace Aws
{
namespace Utils
{
namespace Json
{
  class JsonValue;
  class JsonView;
} // namespace Json
} // namespace Utils
namespace SWF
{
namespace Model
{

  /**
   * <p>Provides the details of the <code>CancelTimerFailed</code>
   * event.</p><p><h3>See Also:</h3>   <a
   * href="http://docs.aws.amazon.com/goto/WebAPI/swf-2012-01-25/CancelTimerFailedEventAttributes">AWS
   * API Reference</a></p>
   */
  class AWS_SWF_API CancelTimerFailedEventAttributes
  {
  public:
    CancelTimerFailedEventAttributes();
    CancelTimerFailedEventAttributes(Aws::Utils::Json::JsonView jsonValue);
    CancelTimerFailedEventAttributes& operator=(Aws::Utils::Json::JsonView jsonValue);
    Aws::Utils::Json::JsonValue Jsonize() const;


    /**
     * <p>The timerId provided in the <code>CancelTimer</code> decision that
     * failed.</p>
     */
    inline const Aws::String& GetTimerId() const{ return m_timerId; }

    /**
     * <p>The timerId provided in the <code>CancelTimer</code> decision that
     * failed.</p>
     */
    inline void SetTimerId(const Aws::String& value) { m_timerIdHasBeenSet = true; m_timerId = value; }

    /**
     * <p>The timerId provided in the <code>CancelTimer</code> decision that
     * failed.</p>
     */
    inline void SetTimerId(Aws::String&& value) { m_timerIdHasBeenSet = true; m_timerId = std::move(value); }

    /**
     * <p>The timerId provided in the <code>CancelTimer</code> decision that
     * failed.</p>
     */
    inline void SetTimerId(const char* value) { m_timerIdHasBeenSet = true; m_timerId.assign(value); }

    /**
     * <p>The timerId provided in the <code>CancelTimer</code> decision that
     * failed.</p>
     */
    inline CancelTimerFailedEventAttributes& WithTimerId(const Aws::String& value) { SetTimerId(value); return *this;}

    /**
     * <p>The timerId provided in the <code>CancelTimer</code> decision that
     * failed.</p>
     */
    inline CancelTimerFailedEventAttributes& WithTimerId(Aws::String&& value) { SetTimerId(std::move(value)); return *this;}

    /**
     * <p>The timerId provided in the <code>CancelTimer</code> decision that
     * failed.</p>
     */
    inline CancelTimerFailedEventAttributes& WithTimerId(const char* value) { SetTimerId(value); return *this;}


    /**
     * <p>The cause of the failure. This information is generated by the system and can
     * be useful for diagnostic purposes.</p> <note> <p>If <code>cause</code> is set to
     * <code>OPERATION_NOT_PERMITTED</code>, the decision failed because it lacked
     * sufficient permissions. For details and example IAM policies, see <a
     * href="http://docs.aws.amazon.com/amazonswf/latest/developerguide/swf-dev-iam.html">Using
     * IAM to Manage Access to Amazon SWF Workflows</a> in the <i>Amazon SWF Developer
     * Guide</i>.</p> </note>
     */
    inline const CancelTimerFailedCause& GetCause() const{ return m_cause; }

    /**
     * <p>The cause of the failure. This information is generated by the system and can
     * be useful for diagnostic purposes.</p> <note> <p>If <code>cause</code> is set to
     * <code>OPERATION_NOT_PERMITTED</code>, the decision failed because it lacked
     * sufficient permissions. For details and example IAM policies, see <a
     * href="http://docs.aws.amazon.com/amazonswf/latest/developerguide/swf-dev-iam.html">Using
     * IAM to Manage Access to Amazon SWF Workflows</a> in the <i>Amazon SWF Developer
     * Guide</i>.</p> </note>
     */
    inline void SetCause(const CancelTimerFailedCause& value) { m_causeHasBeenSet = true; m_cause = value; }

    /**
     * <p>The cause of the failure. This information is generated by the system and can
     * be useful for diagnostic purposes.</p> <note> <p>If <code>cause</code> is set to
     * <code>OPERATION_NOT_PERMITTED</code>, the decision failed because it lacked
     * sufficient permissions. For details and example IAM policies, see <a
     * href="http://docs.aws.amazon.com/amazonswf/latest/developerguide/swf-dev-iam.html">Using
     * IAM to Manage Access to Amazon SWF Workflows</a> in the <i>Amazon SWF Developer
     * Guide</i>.</p> </note>
     */
    inline void SetCause(CancelTimerFailedCause&& value) { m_causeHasBeenSet = true; m_cause = std::move(value); }

    /**
     * <p>The cause of the failure. This information is generated by the system and can
     * be useful for diagnostic purposes.</p> <note> <p>If <code>cause</code> is set to
     * <code>OPERATION_NOT_PERMITTED</code>, the decision failed because it lacked
     * sufficient permissions. For details and example IAM policies, see <a
     * href="http://docs.aws.amazon.com/amazonswf/latest/developerguide/swf-dev-iam.html">Using
     * IAM to Manage Access to Amazon SWF Workflows</a> in the <i>Amazon SWF Developer
     * Guide</i>.</p> </note>
     */
    inline CancelTimerFailedEventAttributes& WithCause(const CancelTimerFailedCause& value) { SetCause(value); return *this;}

    /**
     * <p>The cause of the failure. This information is generated by the system and can
     * be useful for diagnostic purposes.</p> <note> <p>If <code>cause</code> is set to
     * <code>OPERATION_NOT_PERMITTED</code>, the decision failed because it lacked
     * sufficient permissions. For details and example IAM policies, see <a
     * href="http://docs.aws.amazon.com/amazonswf/latest/developerguide/swf-dev-iam.html">Using
     * IAM to Manage Access to Amazon SWF Workflows</a> in the <i>Amazon SWF Developer
     * Guide</i>.</p> </note>
     */
    inline CancelTimerFailedEventAttributes& WithCause(CancelTimerFailedCause&& value) { SetCause(std::move(value)); return *this;}


    /**
     * <p>The ID of the <code>DecisionTaskCompleted</code> event corresponding to the
     * decision task that resulted in the <code>CancelTimer</code> decision to cancel
     * this timer. This information can be useful for diagnosing problems by tracing
     * back the chain of events leading up to this event.</p>
     */
    inline long long GetDecisionTaskCompletedEventId() const{ return m_decisionTaskCompletedEventId; }

    /**
     * <p>The ID of the <code>DecisionTaskCompleted</code> event corresponding to the
     * decision task that resulted in the <code>CancelTimer</code> decision to cancel
     * this timer. This information can be useful for diagnosing problems by tracing
     * back the chain of events leading up to this event.</p>
     */
    inline void SetDecisionTaskCompletedEventId(long long value) { m_decisionTaskCompletedEventIdHasBeenSet = true; m_decisionTaskCompletedEventId = value; }

    /**
     * <p>The ID of the <code>DecisionTaskCompleted</code> event corresponding to the
     * decision task that resulted in the <code>CancelTimer</code> decision to cancel
     * this timer. This information can be useful for diagnosing problems by tracing
     * back the chain of events leading up to this event.</p>
     */
    inline CancelTimerFailedEventAttributes& WithDecisionTaskCompletedEventId(long long value) { SetDecisionTaskCompletedEventId(value); return *this;}

  private:

    Aws::String m_timerId;
    bool m_timerIdHasBeenSet;

    CancelTimerFailedCause m_cause;
    bool m_causeHasBeenSet;

    long long m_decisionTaskCompletedEventId;
    bool m_decisionTaskCompletedEventIdHasBeenSet;
  };

} // namespace Model
} // namespace SWF
} // namespace Aws
