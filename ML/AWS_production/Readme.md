Deploying to AWS lambda
===========================

## Step 1
`docker build -t [image-name]:[version] .`
## Step 2
`docker tag [image-name]:[version] [user-id].dkr.ecr.[region].amazonaws.com/[repo-name]:[version]`
## Step 3
`docker push docker push [user-id].dkr.ecr.[region].amazonaws.com/[repo-name]:latest`